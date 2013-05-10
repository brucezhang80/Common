/*
 * @File: tservfrm.h
 * @Desc: head file of TServer Frame
 * @Author: stevetang
 * @History:
 *      2007-11-26   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _TSERVFRM_H
#define _TSERVFRM_H

#include "tfrm.h"
#include "lthread.h"
#include "base_socket.h"
#include "tcp_server.h"
#include "buffer.h"
#include "hash.h"
#include "tservconf.h"
#include "realtime.h"

/* 协议包 */
#pragma pack(1)

typedef struct _tagTServProtoHead {
	unsigned int length; /* 包长度：包头+包体 */
	unsigned int cmd; /* 命令码 */
	unsigned int sequence; /* 序列号 */
} recTServProtoHead;

typedef struct _tagTServProtoPck {
	recTServProtoHead head; /* 数据头 */
	char data[0]; /* 数据 */
} recTServProtoPck;

#pragma pack()

class CTServBuffer;
class CTServFrm;
class CTSBizThreadPool;

/* 业务处理线程 */
class CTSBizThread : public CLThread
{
	CTServFrm * m_tserv; /* TSERVER对象 */
    CTSBizThreadPool * m_pool; /* 所属对象池 */

	CTServBuffer * m_buff; /* 缓冲对象 */

public:
    CTSBizThread(CTServFrm * tserv, CTSBizThreadPool * pool);
    ~CTSBizThread();

protected:
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

private:
    /*
     * Function: Attach
     * Desc: 绑定句柄
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    int Attach(CTServBuffer * buff);

    /*
     * Function: Detach
     * Desc: 卸载句柄
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    int Detach();

public:
    /*
     * Function: Resume
     * Desc: 唤醒线程
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    int Resume(CTServBuffer * buff);

    /*
     * Function: Pause
     * Desc: 挂起线程
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    int Pause();

    /*
     * Function: GetBuffer
     * Desc: 获取操作缓冲
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    CTServBuffer * GetBuffer() { return m_buff; };
};

class CTSBizThreadPool : public CObjPool<CTSBizThread>
{
    CTServFrm * m_tservfrm; /* 所属TSERVER框架 */
	CHash m_thread_list; /* 线程管理表 */
	int m_isstop; /* 是否退出 */

public:
    CTSBizThreadPool(CTServFrm * frm, int num);
    ~CTSBizThreadPool();

protected:
    /*
     * Function: newobject
     * Desc: 创建对象
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      非NULL  -   成功
     *      NULL    -   失败
     */
    CTSBizThread * newobject();

public:
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
	CTSBizThread* get(CTServBuffer * buff);

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
	int release(CTSBizThread* elem);

    /****************************************************************************** 
     * FUNCTION:  CObjPool.get
     * DESCRIPTION:  获取正在运行的线程对象 
     * Input: pthread_t h
     * Output: 
     * Returns: 
     * 
     * modification history
     * --------------------
     * 01a, 30may2007, nancyyu written
     * --------------------
     ******************************************************************************/
	CTSBizThread * find(pthread_t h);

    /*
     * Function: Stop
     * Desc: 停止所有处理线程
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    int Stop();
};

/* 连接使用缓冲 */
class CTServBuffer
{
	int m_buflen; /* 缓冲最大长度 */

	char * m_request; /* 请求缓冲 */
	int m_reqlen; /* 实际请求数据长度 */
	int m_reqleft; /* 剩余请求数据长度 */

	char * m_response; /* 响应缓冲 */
	int m_rspleft; /* 剩余响应数据长度 */

	CBaseSocket m_sock; /* 对应的连接 */

public:
	CTServBuffer(int buflen);
	~CTServBuffer();

public:
	typedef enum {
		E_IDLE = 0, /* 空闲可用 */
		E_INUSE, /* 存在数据，使用中 */
		E_UNUSED, /* 无用的，废弃的 */
	} enuBuffStatus;

private:
	enuBuffStatus m_status; /* 缓冲状态 */

public:
    /*
     * Function: Attach
     * Desc: 关联TCP句柄
     * In: 
     *      int  sock   TCP句柄
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int Attach(int sock);

    /*
     * Function: Detach
     * Desc: 释放句柄
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int Detach();

    /*
     * Function: Recv
     * Desc: 接收数据
     * In: 
     *      char *     buf  响应数据
	 *      int        len  数据长度
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int Recv(char * buf, int len);

    /*
     * Function: Recv
     * Desc: 接收数据
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int Recv();

    /*
     * Function: Send
     * Desc: 发送数据
     * In: 
     *      const char *   buf  响应数据
	 *      int            len  数据长度
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int Send(const char * buf, int len);

    /*
     * Function: Send
     * Desc: 发送剩余数据
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int Send();

    /*
     * Function: Write
     * Desc: 发送数据
     * In: 
     *      const char *   buf  响应数据
	 *      int            len  数据长度
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int Write(const char * buf, int len);

    /*
     * Function: GetRequest
     * Desc: 返回请求缓冲
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	const char * GetRequest() { return m_request; };

    /*
     * Function: GetRequestLength
     * Desc: 返回请求数据长度
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int GetRequestLength() { return m_reqlen; };

    /*
     * Function: GetSock
     * Desc: 返回关联TCP句柄
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      >0  -   成功
     *      -1 -   失败
     */
	int GetSock() { return m_sock.GetFD(); };

	enuBuffStatus GetStatus() { return m_status; };
	int SetStatus(enuBuffStatus status) { m_status = status; return 0; };

	const char * GetResponse() { return m_response; };
	int GetResponseLeft() { return m_rspleft; };
};

class CTServBufferPool : public CObjPool<CTServBuffer>
{
    int m_buflen; /* 数据长度 */

public:
	CTServBufferPool(int num, int buflen);
	~CTServBufferPool();

protected:
    /*
     * Function: newobject
     * Desc: 创建对象
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      非NULL  -   成功
     *      NULL    -   失败
     */
    CTServBuffer * newobject();

public:
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
	CTServBuffer* get(int sock);

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
	int release(CTServBuffer* elem);
};

/* 连接处理 */
class CTServConn
{
	CTServFrm * m_tserv; /* 宿主对象 */
	int m_fd; /* epoll句柄 */

	int m_max_client; /* 最大客户端数目 */
	struct epoll_event * m_events; /* epoll事件列表 */

public:
	CTServConn(CTServFrm * tserv, int max_client);
	~CTServConn();

public:
    /*
     * Function: Handle
     * Desc: 线程业务处理
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    int Handle();

    /*
     * Function: Add
     * Desc: 增加待处理TCP句柄
     * In: 
     *      int  sock   TCP句柄
	 *      int  flag   事件MASK
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    int Add(int sock, int flag = EPOLLIN |EPOLLOUT|EPOLLERR | EPOLLHUP);

    /*
     * Function: Mod
     * Desc: 修改待处理TCP句柄
     * In: 
     *      int  sock   TCP句柄
	 *      int  flag   事件MASK
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    int Mod(int sock, int flag = EPOLLIN |EPOLLOUT|EPOLLERR | EPOLLHUP);

    /*
     * Function: Remove
     * Desc: 移除待处理TCP句柄
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
    int Remove(int sock);
};

class CTServTimeout: public CRealTime
{
	CTServFrm * m_tserv; /* 属主 */

public:
	CTServTimeout(CTServFrm * tserv, int num, int timeout = -1);
	~CTServTimeout();

private:
    /*
     * Function: HandleTimeout
     * Desc: 超时处理
     * In: 
     *      size_t key
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	virtual int HandleTimeout(size_t key);
};

class CTServFrm : public CTFrm
{
	friend class CTSBizThread;

    CTServConf m_conf; /* 配置数据 */

    CTSBizThreadPool * m_pool; /* 业务处理线程池 */

	CTServBufferPool * m_bufpool; /* 业务处理缓冲池 */
	CHash * m_reqbuf; /* 请求缓冲池,用于存放未接收完的缓冲对象 */
	CHash * m_rspbuf; /* 响应缓冲池,用于存放未发送完的缓冲对象 */

    CSockAcceptor * m_serv; /* TCP SERVER 对象 */
	CTServConn * m_epoller; /* 连接处理 */

	CHash * m_inuse; /* 记载正在处理的句柄 */

	CTServTimeout * m_timeout; /* 超时请求 */

	/* 系统统计信息 */
	unsigned int m_cltnum; /* 客户端数目 */
	unsigned long long m_reqnum; /* 接收到的请求数目 */

public:
    CTServFrm(const char * name, const char * conf);
    ~CTServFrm();

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

    /*
     * Function: HandleClient
     * Desc: 处理客户端信息
     * In: 
     *      const char *   buf     数据缓冲
	 *      int            buflen  数据长度
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      <0    -   失败
     */
    int HandleClient(const char * buf, int buflen);

    /*
     * Function: nTServWrite
     * Desc: 处理响应数据
     * In: 
     *      const char *   buf     数据缓冲
	 *      int            buflen  数据长度
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      <0    -   失败
     */
    static int nTServWrite(const char * buf, int buflen);

private:
    /*
     * Function: AcceptConnect
     * Desc: 接收客户端连接
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      -1    -   失败
     */
    int AcceptConnect();

public:
    /*
     * Function: HandleIn
     * Desc: 处理客户端接收请求
     * In: 
	 *      int            sock  TCP句柄
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      -1    -   失败
     */
    int HandleIn(int sock);

    /*
     * Function: HandleOut
     * Desc: 处理客户端输出
     * In: 
	 *      int            sock  TCP句柄
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      -1    -   失败
     */
    int HandleOut(int sock);

    /*
     * Function: HandleErr
     * Desc: 处理客户端错误
     * In: 
	 *      int            sock  TCP句柄
	 *      CTServBuffer * buff  对应缓冲对象
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      -1    -   失败
     */
    int HandleErr(int sock, CTServBuffer * buff = NULL);
 
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
    int ReloadConf(const char *conf);

    /*
     * Function: HandleOut
     * Desc: 处理客户端输出
     * In: 
	 *      int            sock  TCP句柄
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      -1    -   失败
     */
    int HandleOut(CTServBuffer * buff);

    /*
     * Function: PreProc
     * Desc: 预处理
     * In: 
	 *      int            sock  TCP句柄
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      -1    -   失败
     */
    int PreProc(CTServBuffer * buff);

    /*
     * Function: EndProc
     * Desc: 结束处理
     * In: 
	 *      int            sock  TCP句柄
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      -1    -   失败
     */
    int EndProc(CTServBuffer * buff);

    /*
     * Function: HandleTimeout
     * Desc: 处理超时请求
     * In: 
	 *      int            sock  TCP句柄
     * Out: 
     *      none
     * Return code: 
     *      0     -   成功
     *      -1    -   失败
     */
    int HandleTimeout(int sock);

	/* 获取池接口 */
    CTSBizThreadPool * GetTSBizThreadPool() const { return m_pool; }; /* 业务处理线程池 */
	CTServBufferPool * GetTServBufferPool() const { return m_bufpool; }; /* 业务处理缓冲池 */
	CTServConn * GetTServConn() const { return m_epoller; }; /* 连接处理 */
};

#endif
