/*
 * @File: event.cpp
 * @Desc: implement file of event
 * @Author: stevetang
 * @History:
 *      2006-08-16   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "event.h"

CEvent::CEvent() : m_producer(0), m_customer(0)
{
    //创建互斥量
    if (pthread_mutex_init(&m_mutex, NULL))
        return;

    //创建条件变量
    if (pthread_cond_init(&m_cond, NULL))
        return;
}

CEvent::~CEvent()
{
    //释放互斥量
    pthread_mutex_destroy(&m_mutex);

    //释放条件变量
    pthread_cond_destroy(&m_cond);
}

int CEvent::Wait()
{
    //加锁
    pthread_mutex_lock(&m_mutex);

    while (m_producer <= 0) /* 是否有数据 */
    {
        /* 等待,主要用于广播 */
        m_customer++;

        //等待条件变量
        pthread_cond_wait(&m_cond, &m_mutex);

        /* 已被唤醒 */
        m_customer--;
    }

    /* 生产者减少 */
    m_producer--;

    //释放锁
    pthread_mutex_unlock(&m_mutex);

    return 0;
}

int CEvent::TimeWait(int sec, int nsec)
{
    timespec tm;
    int rv = 0;
    struct timeval t;

    gettimeofday(&t, NULL);

    tm.tv_sec = t.tv_sec + sec;
    tm.tv_nsec = t.tv_usec * 1000 + nsec;
    if (tm.tv_nsec >= 1000 * 1000 * 1000)
    {
        tm.tv_sec++;
        tm.tv_nsec -= (1000 * 1000 * 1000);
    }

    //加锁
    pthread_mutex_lock(&m_mutex);

    while (m_producer <= 0) /* 是否有数据 */
    {
        /* 等待,主要用于广播 */
        m_customer++;

        //等待条件变量
        rv = pthread_cond_timedwait(&m_cond, &m_mutex, &tm);

        /* 已被唤醒 */
        m_customer--;

        if (rv)
        {
            if (rv != ETIMEDOUT)
                rv = -1; //系统错误
            else if (rv == ETIMEDOUT)
                rv = 1; //超时
            break;
        }
    }

    if (rv == 0)
        /* 生产者被消费 */
        m_producer--;

    //释放锁
    pthread_mutex_unlock(&m_mutex);

    return rv;
}

int CEvent::SetEvent()
{
    //加锁
    pthread_mutex_lock(&m_mutex);

    /* 生产者增加 */
    m_producer++;

    //发送条件变量信号
    pthread_cond_signal(&m_cond);

    //释放锁
    pthread_mutex_unlock(&m_mutex);

    return 0;
}

int CEvent::BroadCastEvent()
{
    //加锁
    pthread_mutex_lock(&m_mutex);

    /* 生产者=等待消费的数目 */
    m_producer = m_customer;

    //广播条件变量信号
    pthread_cond_broadcast(&m_cond);

    //释放锁
    pthread_mutex_unlock(&m_mutex);

    return 0;
}
