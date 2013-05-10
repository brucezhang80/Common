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
	CThreadLock m_lock; /* 线程锁 */
    CGenericList<T> m_list; /* 队列 */

    unsigned char m_isblock; /* 是否为阻塞方式 */
    unsigned char m_release; /* 是否为释放标志 */
    CEvent m_hEvent; /* 事件 */
    int m_num; /* 对象数目 */

public:
	CObList(int item = 10, bool isblock = 1) : m_list(item) { 
        m_isblock = isblock; 
        m_release = 0;
        m_num = 0; 
    };

	virtual ~CObList() { /* ???阻塞多线程存在BUG,不易安全退出线程?? */
        if (m_isblock)
        {
            /* 设置事件 */
            WakeUpAll();
        }

        CAutoLock l(&m_lock);

        T * obj = NULL;

        /* 清除队列 */
        while ((obj = m_list.RemoveHead()) != NULL)
        {
            delete obj;
            obj = NULL;
        }
    };

public:
    /*
     * Function: release
     * Desc: 增加对象
     * In: 
     *     T*  obj
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
	int release(T* obj) {
        if (obj == NULL)
        {
            return -1;
        }

        CAutoLock l(&m_lock);

        /* 放入队列 */
        m_list.AddTail(obj);

        if (m_isblock)
        {
            /* 设置事件 */
            m_hEvent.SetEvent();
        }

        /* 计数+1 */
        m_num++;

        return 0;
    };
	
    /*
     * Function: get
     * Desc: 取对象
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
	T* get(int timeout = -1) {
        T * obj = NULL;

		if (m_release)
		{
			/* 对象已释放 */
			return NULL;
		}

        do
        {
            if (m_num <= 0 && m_isblock)
            {
				int rv = 0;
                /* 等待对象 */
				if (timeout == -1)
	                rv = m_hEvent.Wait();
				else
	                rv = m_hEvent.TimeWait(timeout, 0);
                if (m_release || rv)
                {
                    /* 系统退出 */
                    return NULL;
                }
            }

            CAutoLock l(&m_lock);

            /* 队列移出头对象 */
            obj = m_list.RemoveHead();
            if (obj)
            {
                /* 计数-1 */
                m_num--;
            }
        }
        while (obj == NULL && m_isblock);

        return obj;
    };
	
    /*
     * Function: WakeUp
     * Desc: 唤醒
     * In: 
     *     unsigned char  isrelease  是否退出
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
	int WakeUp(unsigned char isrelease = 1) {
        m_release = isrelease;

        /* 发送唤醒事件 */
        m_hEvent.SetEvent();

        return 0;
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
        m_hEvent.BroadCastEvent();

        return 0;
    };

	int GetCount()
	{
		return m_num;
	}
};

#endif
