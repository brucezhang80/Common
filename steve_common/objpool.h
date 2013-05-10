/******************************************************************************
 * Copyright:      Shenzhen Tencent Co.Ltd.
 * 
 * ObjPool.h 
 * 
 * Description:  Head file of an object pool
 * 
 * Modification history
 * --------------------
 * 01a, 24may2007, nancyyu create
 * 2007-6-5, stevetang modify, list/idle/busy...
 * --------------------
 ******************************************************************************/
#ifndef __OBJPOOL_H_
#define __OBJPOOL_H_

#include "lock.h"
#include "baselist.h"
#include "hash.h"
#include "event.h"

template <class T> 

class CObjPool{
public:
	CObjPool(int num, unsigned char block = 0) : m_idle(num), m_busy(num), m_isblock(block), m_release(0) {
        m_num = num; /* ������Ŀ */
        m_idle_num = num; /* ���ж�����Ŀ */
    };

	virtual ~CObjPool() {
        if (m_isblock)
        {
			/* �ͷ� */
			m_release = 1;

            /* �����¼� */
            m_event.BroadCastEvent();
        }

        CAutoLock l(&m_lock);

        T * obj = NULL;

        /* ���æ���� */
        while (1)
        {
			int bucket = -1;
            obj = (T *)m_busy.RemoveHead(bucket);
            if (obj == NULL)
                break;
            delete obj;
            obj = NULL;
        }

        /* ������ж��� */
        while (1)
        {
            obj = m_idle.RemoveHead();
            if (obj == NULL)
                break;
            delete obj;
            obj = NULL;
        }
    };

protected:
	CThreadLock m_lock; /* �߳��� */
    CGenericList<T> m_idle; /* �ж��� */
    CHash m_busy; /* æ���� */
	int m_num; /* ������Ŀ */
    int m_idle_num; /* ���ж�����Ŀ */

	unsigned char m_isblock; /* �Ƿ�Ϊ������ʽ */
    unsigned char m_release; /* �Ƿ�Ϊ�ͷű�־ */
	CEvent m_event; /* �¼� */

public:
    virtual T* newobject() = 0;
	
public:
    /****************************************************************************** 
     * FUNCTION:  CObjPool.release 
     * DESCRIPTION:  release object,and push it to the idle queue 
     * Input: T& elem, an object quote
     * Output: 
     * Returns: 
     * 
     * modification history
     * --------------------
     * 01a, 30may2007, nancyyu written
     * --------------------
     ******************************************************************************/
	int init() {
        /* ������ */
        for (int i=0; i<m_num; i++)
        {
            T * obj = newobject();
            if (obj)
            {
                /* ������ж��� */
                m_idle.AddTail(obj);
            }
        }

        return 0;
    }

    /****************************************************************************** 
     * FUNCTION:  CObjPool.release 
     * DESCRIPTION:  release object,and push it to the idle queue 
     * Input: T& elem, an object quote
     * Output: 
     * Returns: 
     * 
     * modification history
     * --------------------
     * 01a, 30may2007, nancyyu written
     * --------------------
     ******************************************************************************/
	int release(T* elem) {
        if (elem == NULL)
        {
            return -1;
        }

        CAutoLock l(&m_lock);

        /* æ�������Ƴ����� */
        m_busy.Remove((size_t)elem);

        /* ������ж��� */
        m_idle.AddTail(elem);

        if (m_isblock)
        {
            /* �����¼� */
            m_event.SetEvent();
        }

        m_idle_num++;

        return 0;
    };
	
    /****************************************************************************** 
     * FUNCTION:  CObjPool.get 
     * DESCRIPTION:  get object from the idle queue 
     * Input: 
     * Output: an object quote
     * Returns: 
     * 
     * modification history
     * --------------------
     * 01a, 30may2007, nancyyu written
     * --------------------
     ******************************************************************************/
	T* get(int timeout = -1) {
        T * elem = NULL;

		do
		{
            if (m_idle_num <= 0 && m_isblock)
            {
				int rv = 0;
                /* �ȴ����� */
				if (timeout < 0)
				{
	                rv = m_event.Wait();
				}
				else
				{
					int sec = timeout / 1000;
					int nsec = (timeout % 1000) * 1000;
	                rv = m_event.TimeWait(sec, nsec);
				}

                if (m_release || rv)
                {
                    /* ϵͳ�˳� */
                    return NULL;
                }
            }

			CAutoLock l(&m_lock);

			/* �ж����Ƴ�ͷ���� */
			elem = m_idle.RemoveHead();
			if (elem)
			{
				/* ����æ���� */
				m_busy.Add((size_t)elem, elem);

				m_idle_num--;
			}
		}
		while (elem == NULL && m_isblock);

        return elem;
    };

    /*
     * Function: WakeUpAll
     * Desc: ��������
     * In: 
     *     unsigned char  isrelease  �Ƿ��˳�
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
	int WakeUpAll(unsigned char isrelease = 1) {
        m_release = isrelease;

        /* ���ͻ����¼� */
        m_event.BroadCastEvent();

        return 0;
    };

    /****************************************************************************** 
     * FUNCTION:  CObjPool.get_num 
     * DESCRIPTION:  get object number of object pool 
     * Input: 
     * Output: object number
     * Returns: object number
     * 
     * modification history
     * --------------------
     * 01a, 30may2007, nancyyu written
     * --------------------
     ******************************************************************************/
	int get_num() { return m_num; };

    /****************************************************************************** 
     * FUNCTION:  CObjPool.get_num 
     * DESCRIPTION:  get object number of object pool 
     * Input: 
     * Output: object number
     * Returns: object number
     * 
     * modification history
     * --------------------
     * 01a, 30may2007, nancyyu written
     * --------------------
     ******************************************************************************/
	int get_idle_num() { CAutoLock l(&m_lock); return m_idle_num; };
};

#endif
