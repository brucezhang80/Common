/*
 * @File: lthread.cpp
 * @Desc: implement file of thread interface
 * @Author: stevetang
 * @History:
 *      2007-11-23   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "lthread.h"

CLThread::CLThread(const char * name /* = "" */, unsigned char hangup /* = 0 */, int timeout /* = -1 */) : 
m_running(0), m_hangup(hangup), m_timeout(timeout)
{
	strcpy(m_name, name);
}

CLThread::~CLThread()
{
    /* �ر��߳� */
    Stop();
}

/*
 * Function: Start
 * Desc: �����߳�
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CLThread::Start()
{
    if (m_running)
    {
        /* �Ѿ����� */
        return 1;
    }

    /* �����߳����� */
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024 * 1024);

    int ret = pthread_create(&m_hThread, &attr, CLThread::ThreadEntry, this);
    if (ret)
    {
        return -1;
    }

//	printf("%s:%u start.\n", m_name, (unsigned int)m_hThread);

    return 0;
}

/*
 * Function: Stop
 * Desc: ֹͣ�߳�
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CLThread::Stop()
{
    if (m_running == 0)
    {
        /* �Ѿ�ֹͣ */
        return 1;
    }

    /* �߳��˳� */
    m_running = 0;

    /* ���ѵȴ��߳� */
    WakeUp();

    /* ֹͣ�¼� */
    OnStop();

//	printf("%s:%u stop wait.\n", m_name, (unsigned int)m_hThread);

    /* �ȴ��߳��˳� */
    pthread_join(m_hThread, NULL);

//	printf("%s:%u stop ok.\n", m_name, (unsigned int)m_hThread);

    return 0;
}

/*
 * Function: ThreadDispatch
 * Desc: �߳�ҵ������
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CLThread::ThreadDispatch()
{
	int sec = 0, nsec = 0;
	if (m_timeout > 0)
	{
		sec = m_timeout/1000;
		nsec = (m_timeout%1000) * 1000 * 1000;
	}

    while (m_running)
    {
        int ibreak = 0; /* �Ƿ��˳��߳� */

        if (m_hangup)
        {
			if (m_timeout == -1)
	            m_hEvent.Wait(); /* ����ȴ����� */
			else
	            m_hEvent.TimeWait(sec, nsec); /* ����ȴ����� */
            if (m_running == 0)
            {
                break;
            }
        }

        /* �̵߳�ҵ���� */
        HandleThread(ibreak);
        if (ibreak) /* �˳��߳� */
        {
            break;
        }
    }

	m_running = 0;

    pthread_exit((void*)0);

    return 0;
}

/*
 * Function: ThreadEntry
 * Desc: �����߳�
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
void * CLThread::ThreadEntry(void * argv)
{
    CLThread * lpThis =  (CLThread *)argv;

    lpThis->m_running = 1;

    lpThis->ThreadDispatch();

    return 0;
}

/*
 * Function: IsRunning
 * Desc: �Ƿ��˳�
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
bool CLThread::IsRunning()
{
    return (m_running == 1);
}

/*
 * Function: WakeUp
 * Desc: ����
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CLThread::WakeUp()
{
    if (m_hangup)
    {
        m_hEvent.SetEvent();
    }

    return 0;
}

/*
 * Function: OnStop
 * Desc: ֹͣ�¼�
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CLThread::OnStop()
{
    return 0;
}
