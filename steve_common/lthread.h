/*
 * @File: lthread.h
 * @Desc: head file of thread interface
 * @Author: stevetang
 * @History:
 *      2007-11-23   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _LTHREAD_H
#define _LTHREAD_H

#include "event.h"

class CLThread
{
	char m_name[128]; /* 线程名字 */

    unsigned char m_running; /* 是否正在运行 */
    pthread_t m_hThread; /* 线程句柄 */

    unsigned char m_hangup; /* 是否为等待唤醒类线程 */
    CEvent m_hEvent; /* 事件 */

	int m_timeout; /* 超时时间,毫秒 */

public:
    CLThread(const char * name = "", unsigned char hangup = 0, int timeout = -1);
    virtual ~CLThread();

protected:
    /*
     * Function: HandleThread
     * Desc: 线程业务处理
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    virtual int HandleThread(int& ibreak) = 0;

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
    virtual int OnStop();

private:
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
    int ThreadDispatch();

    /*
     * Function: ThreadEntry
     * Desc: 线程处理函数
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    static void * ThreadEntry(void * argv);

public:
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
    int Start();

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
    int Stop();

    /*
     * Function: IsRunning
     * Desc: 是否退出
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      true  -   成功
     *      false -   失败
     */
    bool IsRunning();

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
    int WakeUp();

    /*
     * Function: GetHandle
     * Desc: 获取线程句柄
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      非NULL  -   成功
     *      NULL    -   失败
     */
    pthread_t GetHandle() { return m_hThread; };

    /*
     * Function: GetName
     * Desc: 获取线程名称
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      非NULL  -   成功
     *      NULL    -   失败
     */
    const char * GetName() { return (const char *)m_name; };
};

#endif
