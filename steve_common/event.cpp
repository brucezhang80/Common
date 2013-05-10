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
    //����������
    if (pthread_mutex_init(&m_mutex, NULL))
        return;

    //������������
    if (pthread_cond_init(&m_cond, NULL))
        return;
}

CEvent::~CEvent()
{
    //�ͷŻ�����
    pthread_mutex_destroy(&m_mutex);

    //�ͷ���������
    pthread_cond_destroy(&m_cond);
}

int CEvent::Wait()
{
    //����
    pthread_mutex_lock(&m_mutex);

    while (m_producer <= 0) /* �Ƿ������� */
    {
        /* �ȴ�,��Ҫ���ڹ㲥 */
        m_customer++;

        //�ȴ���������
        pthread_cond_wait(&m_cond, &m_mutex);

        /* �ѱ����� */
        m_customer--;
    }

    /* �����߼��� */
    m_producer--;

    //�ͷ���
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

    //����
    pthread_mutex_lock(&m_mutex);

    while (m_producer <= 0) /* �Ƿ������� */
    {
        /* �ȴ�,��Ҫ���ڹ㲥 */
        m_customer++;

        //�ȴ���������
        rv = pthread_cond_timedwait(&m_cond, &m_mutex, &tm);

        /* �ѱ����� */
        m_customer--;

        if (rv)
        {
            if (rv != ETIMEDOUT)
                rv = -1; //ϵͳ����
            else if (rv == ETIMEDOUT)
                rv = 1; //��ʱ
            break;
        }
    }

    if (rv == 0)
        /* �����߱����� */
        m_producer--;

    //�ͷ���
    pthread_mutex_unlock(&m_mutex);

    return rv;
}

int CEvent::SetEvent()
{
    //����
    pthread_mutex_lock(&m_mutex);

    /* ���������� */
    m_producer++;

    //�������������ź�
    pthread_cond_signal(&m_cond);

    //�ͷ���
    pthread_mutex_unlock(&m_mutex);

    return 0;
}

int CEvent::BroadCastEvent()
{
    //����
    pthread_mutex_lock(&m_mutex);

    /* ������=�ȴ����ѵ���Ŀ */
    m_producer = m_customer;

    //�㲥���������ź�
    pthread_cond_broadcast(&m_cond);

    //�ͷ���
    pthread_mutex_unlock(&m_mutex);

    return 0;
}
