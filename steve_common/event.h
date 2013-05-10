/*
 * @File: event.h
 * @Desc: head file of event
 * @Author: stevetang
 * @History:
 *      2006-08-16   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _EVENT_H
#define _EVENT_H


#include <pthread.h>

class CEvent
{
    pthread_mutex_t m_mutex; //互斥
    pthread_cond_t m_cond;   //条件变量

    int m_producer; /* 生产者 */
    int m_customer; /* 消费者 */

public:
    CEvent();
    ~CEvent();

public:
    int Wait(); //等待事件发生
    int TimeWait(int sec, int nsec); //超时等待事件发生
    int SetEvent(); //发送事件信号
    int BroadCastEvent(); //广播事件信号
};

#endif
