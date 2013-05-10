/*
 * @File: tfrm.h
 * @Desc: head file of Process Frame
 * @Author: stevetang
 * @History:
 *      2007-11-23   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _TFRM_H
#define _TFRM_H

#include "event.h"
#include "lthread.h"

class CTFrm : public CLThread
{
    CEvent m_event; /* 退出事件 */

    char m_fname[512]; /* 配置文件 */
    int m_fd; /* 进程号文件句柄 */

    char m_name[512]; /* SERVER名称 */
    int m_daemon; /* 是否后台运行 */

    unsigned char m_islock; /* 是否单进程 */

public:
    CTFrm(const char * name, const char * conf, unsigned char islock = 1);
    ~CTFrm();

private:
    /*
     * Function: Usage
     * Desc: 用法说明
     * In: 
     *     const char *   name   进程名
     * Out: 
     *     none
     * Return code:
     *      -1         出错
     *       0         成功
     */
    int Usage(const char * name);

    /*
     * Function: ParseCmd
     * Desc: 解析后续命令
     * In: 
     *      int    argc    参数数目
     *      char * argv[]  命令参数
     * Out: 
     *     none
     * Return code:
     *      -1         出错
     *       0         成功
     */
    int ParseCmd(int argc, char * argv[]);

    /*
     * Function: DispatchCmd
     * Desc: 进程命令
     * In: 
     *      int    argc    参数数目
     *      char * argv[]  命令参数
     * Out: 
     *     none
     * Return code:
     *      -1         出错
     *       0         成功
     */
    int DispatchCmd(int argc, char * argv[]);

    /*
     * Function: DaemonInit
     * Desc: 设置为后台进程
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     none
     */
    void DaemonInit();

    /*
     * Function: IsExit
     * Desc: 进程是否已退出
     * In: 
     *     const char *   fname   进程名
     * Out: 
     *     none
     * Return code:
     *      -1         出错
     *       0         成功
     */
    int IsExit(const char * fname);

    /*
     * Function: CmdStop
     * Desc: 进程命令
     * In: 
     *     const char *   fname   进程名
     * Out: 
     *     none
     * Return code:
     *      -1         出错
     *       0         成功
     */
    int CmdStop(const char * fname);

    /*
     * Function: CmdReload
     * Desc:重新加载配置
     * In: 
     *     const char *   fname   进程名
     * Out: 
     *     none
     * Return code:
     *      -1         出错
     *       0         成功
     */
    int CmdReload(const char * fname);

    /*
     * Function: LockServ
     * Desc: 检查进程是否已启动
     * In: 
     *     const char *   fname   进程名
     * Out: 
     *     none
     * Return code:
     *      -1         出错
     *       0         成功
     */
    int LockServ(const char * fname);

    /*
     * Function: UnLockServ
     * Desc: 释放系统资源
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *      -1         出错
     *       0         成功
     */
    int UnLockServ();

    /*
     * Function: HandleUSR1
     * Desc: SIGUSR1信号处理函数
     * In: 
     *     int    sig    信号
     * Out: 
     *     none
     * Return code:
     *     none
     */
    static void HandleUSR1(int sig);

    /*
     * Function: HandleUSR2
     * Desc: SIGUSR2信号处理函数
     * In: 
     *     int    sig    信号
     * Out: 
     *     none
     * Return code:
     *     none
     */
    static void HandleUSR2(int sig);

protected:
    /*
     * Function: SetProcName
     * Desc: 设置进程名字
     * In: 
     *      int    argc    参数数目
     *      char * argv[]  命令参数
     * Out: 
     *     none
     * Return code:
     *      -1         出错
     *       0         成功
     */
    virtual int SetProcName(int argc, char * argv[]);

    /*
     * Function: InitConf
     * Desc: 初始化配置参数
     * In: 
     *     const char *   conf  配置文件名
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    virtual int InitConf(const char * conf);

    /*
     * Function: InitFrame
     * Desc: 系统初始化
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    virtual int InitFrame();

    /*
     * Function: ReleaseFrame
     * Desc: 系统资源销毁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    virtual int ReleaseFrame();

    /*
     * Function: HandleThread
     * Desc: 线程业务处理
     * In: 
     *      int    ibreak   中断服务标志
     * Out: 
     *      int    ibreak   中断服务标志
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    virtual int HandleThread(int& ibreak);

public:
    /*
     * Function: Start
     * Desc: 启动框架
     * In: 
     *      int    argc    参数数目
     *      char * argv[]  命令参数
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    int Start(int argc, char *argv[]);

    /*
     * Function: Stop
     * Desc: 退出框架
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Stop();

    /*
     * Function: ReloadConf
     * Desc: 重新加载配置文件
     * In: 
     *     const char *   conf  配置文件名
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    virtual int ReloadConf(const char *conf);
};

#endif
