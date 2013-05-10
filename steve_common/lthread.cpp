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
    /* 关闭线程 */
    Stop();
}

/*
 * Function: Start
 * Desc: 启动线程
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CLThread::Start()
{
    if (m_running)
    {
        /* 已经启动 */
        return 1;
    }

    /* 设置线程属性 */
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
 * Desc: 停止线程
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CLThread::Stop()
{
    if (m_running == 0)
    {
        /* 已经停止 */
        return 1;
    }

    /* 线程退出 */
    m_running = 0;

    /* 唤醒等待线程 */
    WakeUp();

    /* 停止事件 */
    OnStop();

//	printf("%s:%u stop wait.\n", m_name, (unsigned int)m_hThread);

    /* 等待线程退出 */
    pthread_join(m_hThread, NULL);

//	printf("%s:%u stop ok.\n", m_name, (unsigned int)m_hThread);

    return 0;
}

/*
 * Function: ThreadDispatch
 * Desc: 线程业务处理函数
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
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
        int ibreak = 0; /* 是否退出线程 */

        if (m_hangup)
        {
			if (m_timeout == -1)
	            m_hEvent.Wait(); /* 挂起等待唤醒 */
			else
	            m_hEvent.TimeWait(sec, nsec); /* 挂起等待唤醒 */
            if (m_running == 0)
            {
                break;
            }
        }

        /* 线程的业务处理 */
        HandleThread(ibreak);
        if (ibreak) /* 退出线程 */
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
 * Desc: 启动线程
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
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
 * Desc: 是否退出
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
bool CLThread::IsRunning()
{
    return (m_running == 1);
}

/*
 * Function: WakeUp
 * Desc: 唤醒
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
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
 * Desc: 停止事件
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CLThread::OnStop()
{
    return 0;
}
