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
    pthread_mutex_t m_mutex; //����
    pthread_cond_t m_cond;   //��������

    int m_producer; /* ������ */
    int m_customer; /* ������ */

public:
    CEvent();
    ~CEvent();

public:
    int Wait(); //�ȴ��¼�����
    int TimeWait(int sec, int nsec); //��ʱ�ȴ��¼�����
    int SetEvent(); //�����¼��ź�
    int BroadCastEvent(); //�㲥�¼��ź�
};

#endif
