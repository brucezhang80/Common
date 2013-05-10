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
        m_num = num; /* 对象数目 */
        m_idle_num = num; /* 空闲对象数目 */
    };

	virtual ~CObjPool() {
        if (m_isblock)
        {
			/* 释放 */
			m_release = 1;

            /* 设置事件 */
            m_event.BroadCastEvent();
        }

        CAutoLock l(&m_lock);

        T * obj = NULL;

        /* 清除忙队列 */
        while (1)
        {
			int bucket = -1;
            obj = (T *)m_busy.RemoveHead(bucket);
            if (obj == NULL)
                break;
            delete obj;
            obj = NULL;
        }

        /* 清除空闲队列 */
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
	CThreadLock m_lock; /* 线程锁 */
    CGenericList<T> m_idle; /* 闲队列 */
    CHash m_busy; /* 忙队列 */
	int m_num; /* 对象数目 */
    int m_idle_num; /* 空闲对象数目 */

	unsigned char m_isblock; /* 是否为阻塞方式 */
    unsigned char m_release; /* 是否为释放标志 */
	CEvent m_event; /* 事件 */

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
        /* 创建池 */
        for (int i=0; i<m_num; i++)
        {
            T * obj = newobject();
            if (obj)
            {
                /* 放入空闲队列 */
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

        /* 忙队列中移出对象 */
        m_busy.Remove((size_t)elem);

        /* 放入空闲队列 */
        m_idle.AddTail(elem);

        if (m_isblock)
        {
            /* 设置事件 */
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
                /* 等待对象 */
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
                    /* 系统退出 */
                    return NULL;
                }
            }

			CAutoLock l(&m_lock);

			/* 闲队列移出头对象 */
			elem = m_idle.RemoveHead();
			if (elem)
			{
				/* 放入忙队列 */
				m_busy.Add((size_t)elem, elem);

				m_idle_num--;
			}
		}
		while (elem == NULL && m_isblock);

        return elem;
    };

    /*
     * Function: WakeUpAll
     * Desc: 唤醒所有
     * In: 
     *     unsigned char  isrelease  是否退出
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
	int WakeUpAll(unsigned char isrelease = 1) {
        m_release = isrelease;

        /* 发送唤醒事件 */
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
