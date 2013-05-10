/*
 * @File: oblist.h
 * @Desc: head file of object list
 * @Author: stevetang
 * @History:
 *      2007-11-23   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef __OBLIST_H
#define __OBLIST_H

#include "lock.h"
#include "event.h"
#include "baselist.h"

template <class T> 

class CObList {
	CThreadLock m_lock; /* �߳��� */
    CGenericList<T> m_list; /* ���� */

    unsigned char m_isblock; /* �Ƿ�Ϊ������ʽ */
    unsigned char m_release; /* �Ƿ�Ϊ�ͷű�־ */
    CEvent m_hEvent; /* �¼� */
    int m_num; /* ������Ŀ */

public:
	CObList(int item = 10, bool isblock = 1) : m_list(item) { 
        m_isblock = isblock; 
        m_release = 0;
        m_num = 0; 
    };

	virtual ~CObList() { /* ???�������̴߳���BUG,���װ�ȫ�˳��߳�?? */
        if (m_isblock)
        {
            /* �����¼� */
            WakeUpAll();
        }

        CAutoLock l(&m_lock);

        T * obj = NULL;

        /* ������� */
        while ((obj = m_list.RemoveHead()) != NULL)
        {
            delete obj;
            obj = NULL;
        }
    };

public:
    /*
     * Function: release
     * Desc: ���Ӷ���
     * In: 
     *     T*  obj
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
	int release(T* obj) {
        if (obj == NULL)
        {
            return -1;
        }

        CAutoLock l(&m_lock);

        /* ������� */
        m_list.AddTail(obj);

        if (m_isblock)
        {
            /* �����¼� */
            m_hEvent.SetEvent();
        }

        /* ����+1 */
        m_num++;

        return 0;
    };
	
    /*
     * Function: get
     * Desc: ȡ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
	T* get(int timeout = -1) {
        T * obj = NULL;

		if (m_release)
		{
			/* �������ͷ� */
			return NULL;
		}

        do
        {
            if (m_num <= 0 && m_isblock)
            {
				int rv = 0;
                /* �ȴ����� */
				if (timeout == -1)
	                rv = m_hEvent.Wait();
				else
	                rv = m_hEvent.TimeWait(timeout, 0);
                if (m_release || rv)
                {
                    /* ϵͳ�˳� */
                    return NULL;
                }
            }

            CAutoLock l(&m_lock);

            /* �����Ƴ�ͷ���� */
            obj = m_list.RemoveHead();
            if (obj)
            {
                /* ����-1 */
                m_num--;
            }
        }
        while (obj == NULL && m_isblock);

        return obj;
    };
	
    /*
     * Function: WakeUp
     * Desc: ����
     * In: 
     *     unsigned char  isrelease  �Ƿ��˳�
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
	int WakeUp(unsigned char isrelease = 1) {
        m_release = isrelease;

        /* ���ͻ����¼� */
        m_hEvent.SetEvent();

        return 0;
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
        m_hEvent.BroadCastEvent();

        return 0;
    };

	int GetCount()
	{
		return m_num;
	}
};

#endif
