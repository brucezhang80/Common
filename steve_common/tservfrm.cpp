/*
 * @File: tservfrm.cpp
 * @Desc: implement file of TServer Frame
 * @Author: stevetang
 * @History:
 *  2007-11-26   Create
 * @Copyright: Shenzhen Tecent Co.Ltd.
 */


#include "general.h"
#include "tservfrm.h"
#include "log.h"

static CTServFrm * g_tservFrm = NULL;

CTSBizThread::CTSBizThread(CTServFrm * tserv, CTSBizThreadPool * pool) :
CLThread("TSBizThread", 1), m_tserv(tserv), m_pool(pool), m_buff(NULL)
{
    Start(); /* 启动线程 */
}

CTSBizThread::~CTSBizThread()
{
}

/*
 * Function: HandleThread
 * Desc: 线程业务处理
 * In:
 *      int    ibreak   中断服务标志
 * Out:
 *      int    ibreak   中断服务标志
 * Return code:
 *      -1      出错
 *       0      成功
 */
int CTSBizThread::HandleThread(int& ibreak)
{
	if (m_buff == NULL)
	{
		return -1;
	}

	/* 处理数据 */
	m_tserv->HandleClient(m_buff->GetRequest(), m_buff->GetRequestLength());

	LOG_TXT_INFO("handle client over;");

	/* 处理结束 */
	m_tserv->EndProc(m_buff);

	LOG_TXT_INFO("end proc over;");

	/* 释放对象，置于空闲状态，等待新的连接 */
    m_pool->release(this);

    LOG_TXT_INFO("release pool over.");

    return 0;
}

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
int CTSBizThread::Attach(CTServBuffer * buff)
{
	m_buff = buff;

    return 0;
}

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
int CTSBizThread::Detach()
{
    m_buff = NULL;

    return 0;
}

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
int CTSBizThread::Resume(CTServBuffer * buff)
{
    /* 绑定句柄 */
    Attach(buff);

	/* 进行预处理 */
	m_tserv->PreProc(buff);

    /* 唤醒线程 */
    WakeUp();

    return 0;
}

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
int CTSBizThread::Pause()
{
    Detach();

    return 0;
}

CTSBizThreadPool::CTSBizThreadPool(CTServFrm * frm, int num) :
    CObjPool<CTSBizThread>(num, 1), m_tservfrm(frm), m_thread_list(num), m_isstop(0)
{
    init();
}

CTSBizThreadPool::~CTSBizThreadPool()
{
}

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
CTSBizThread * CTSBizThreadPool::newobject()
{
    CTSBizThread * pthread = NULL;

    pthread = new CTSBizThread(m_tservfrm, this);
	if (pthread == NULL)
		return NULL;

	m_thread_list.Add((unsigned int)pthread->GetHandle(), pthread);

    return pthread;
}

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
CTSBizThread* CTSBizThreadPool::get(CTServBuffer * buff)
{
    CTSBizThread * pthread = CObjPool<CTSBizThread>::get();
    if (pthread)
    {
        /* 唤醒线程 */
        pthread->Resume(buff);
    }

    return pthread;
}

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
int CTSBizThreadPool::release(CTSBizThread* elem)
{
    /* 暂停线程 */
    elem->Pause();

    return CObjPool<CTSBizThread>::release(elem);
}

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
CTSBizThread * CTSBizThreadPool::find(pthread_t h)
{
	/* 是否退出 */
	if (m_isstop)
		return NULL;

	return (CTSBizThread *)(m_thread_list.Get((unsigned int)h));
}

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
int CTSBizThreadPool::Stop()
{
	/* 设置退出标志 */
	m_isstop = 1;

	/* 终止所有线程 */
	if (m_thread_list.EnumBegin())
		return -1;

	while (1)
	{
		CTSBizThread * thread = (CTSBizThread *)m_thread_list.EnumNext();
		if (thread == NULL)
			break;

		thread->Stop();
	}

	m_thread_list.EnumEnd();

	return 0;
}

CTServBuffer::CTServBuffer(int buflen) :
m_buflen(buflen), m_reqlen(0), m_reqleft(0), m_rspleft(0), m_status(E_IDLE)
{
	m_request = (char *)malloc(m_buflen);
	m_response = (char *)malloc(m_buflen);
}

CTServBuffer::~CTServBuffer()
{
	if (m_request)
	{
		free(m_request);
		m_request = NULL;
	}

	if (m_response)
	{
		free(m_response);
		m_response = NULL;
	}
}

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
int CTServBuffer::Attach(int sock)
{
	m_sock.Attach(sock, SOCK_STREAM);

	/* 数据清空 */
	m_reqlen = m_reqleft = m_rspleft = 0;
	m_status = E_IDLE;

	return 0;
}

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
int CTServBuffer::Detach()
{
	m_sock.Dettach();

	return 0;
}

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
int CTServBuffer::Recv(char * buf, int len)
{
	int left = len;
	char * ptr = buf;

	/* 句柄为非阻塞 */
	while (left > 0)
	{
		/* 接收数据 */
		int ret = m_sock.Recv(ptr, left);
		if (ret <= 0)
		{
			if (ret == -1 && errno == EAGAIN)
				/* 冇数据了 */
				break;

			if (ret == -1 && errno == EINTR)
				/* 被信号中断，还有数据 */
				continue;

			return -1;
		}

		left -= ret;
		ptr += ret;
	}

	/* 返回实际读取的数据 */
	return (len - left);
}

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
int CTServBuffer::Recv()
{
	if (m_request == NULL || !m_sock.IsValid())
		return -1;

	/* 接收数据 */
	char * ptr = m_request + m_reqlen - m_reqleft;
	int left = m_buflen - (m_reqlen - m_reqleft);
	int ret = Recv(ptr, left);
	if (ret <= 0 || ret < (int)sizeof(unsigned int)) /* 4个字节也没收到记为错误算了 */
	{
		return -1;
	}

	/* 检查数据 */
	int len = m_reqlen - m_reqleft + ret;

	/* 取数据长度 */
	memcpy(&m_reqlen, m_request, sizeof(unsigned int));

	/* 存在错误数据 */
	if (len > m_reqlen)
	{
		LOG_TXT_ERR("invalid request.[%d]", m_sock.GetFD());
		return -1;
	}

	m_reqleft = m_reqlen - len;

	if (m_reqleft > 0)
	{
		/* 还存在数据 */
		return 1;
	}

	LOG_BIN_DEBUG(m_request, m_reqlen);

	return 0;
}

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
int CTServBuffer::Send(const char * buf, int len)
{
	if (m_response == NULL || !m_sock.IsValid())
		return -1;

	/* 发送数据 */
	int left = len;
	char * ptr = (char *)buf;

	while (left > 0)
	{
		int ret = m_sock.Send(ptr, left);
		if (ret <= 0)
		{
			if (errno == EAGAIN)
				/* 冇缓冲了 */
				break;

			if (errno == EINTR)
				/* 被信号中断，还可发 */
				continue;

			return -1;
		}

		left -= ret;
		ptr += ret;
	}

	LOG_TXT_DEBUG("Send:len: %d left: %d", len, left);

	LOG_BIN_DEBUG(buf, len - left);

	/* 返回实际发送数据大小 */
	return (len - left);
}

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
int CTServBuffer::Send()
{
	if (m_response == NULL || !m_sock.IsValid())
		return -1;

	/* 发送数据 */
	int ret = Send(m_response, m_rspleft);
	if (ret < 0)
	{
		return -1;
	}

	/* 确认是否还存在剩余数据 */
	m_rspleft -= ret;
	if (m_rspleft > 0)
	{
		/* 还存在待发送数据,复制剩余数据 */
		char * ptr = m_response + ret;
		memcpy(m_response, ptr, m_rspleft);
		m_status = E_INUSE; /* 存在数据 */
		return 1;
	}

	m_status = E_IDLE; /* 数据已全部清空 */

	return 0;
}

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
int CTServBuffer::Write(const char * buf, int len)
{
	if (m_response == NULL || m_status == E_UNUSED)
		return -1;

	/* 检查是否存在未完成数据 */
	if (m_rspleft > 0)
	{
		/* 先发送早期数据 */
		int ret = Send();
		if (ret < 0)
			return -1;

		if (ret > 0)
		{
			/* 存在未发送数据 */
			char * ptr = m_response + m_rspleft;
			if ((len + m_rspleft) >= m_buflen)
				return -2;
			memcpy(ptr, buf, len);
			m_rspleft += len;
			m_status = E_INUSE; /* 存在数据 */
			return 1;
		}
	}

	/* 发送数据 */
	int ret = Send(buf, len);
	if (ret < 0)
		return -1;

	/* 确认是否存在剩余 */
	m_rspleft = len - ret;
	if (m_rspleft > 0)
	{
		/* 存在剩余数据,备份哈 */
		char * ptr = (char *)buf + ret;
		memcpy(m_response, ptr, m_rspleft);
		m_status = E_INUSE; /* 存在数据 */
		return 1;
	}

	m_status = E_IDLE; /* 数据已全部清空 */

	return 0;
}

CTServBufferPool::CTServBufferPool(int num, int buflen) :
CObjPool<CTServBuffer>(num), m_buflen(buflen)
{
    /* 初始化对象池 */
    init();
}

CTServBufferPool::~CTServBufferPool()
{
}

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
CTServBuffer * CTServBufferPool::newobject()
{
    CTServBuffer * buff = NULL;

    buff = new CTServBuffer(m_buflen);
	if (buff == NULL)
		return NULL;

    return buff;
}

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
CTServBuffer* CTServBufferPool::get(int sock)
{
    CTServBuffer * buff = CObjPool<CTServBuffer>::get();
	if (buff == NULL)
		return NULL;

	/* 绑定TCP对象 */
	buff->Attach(sock);

	return buff;
}

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
int CTServBufferPool::release(CTServBuffer* elem)
{
	/* 释放TCP句柄 */
	elem->Detach();

    return CObjPool<CTServBuffer>::release(elem);
}

CTServConn::CTServConn(CTServFrm * tserv, int max_client) :
m_tserv(tserv), m_fd(-1), m_max_client(max_client), m_events(NULL)
{
	/* 创建epoll */
	int size =  max_client < 1024 ? 1024 : max_client;
	m_fd = epoll_create(size);
	if (m_fd == -1)
		return;

	/* 确定最大存放事件空间 */
	m_events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * (size + 1024));
	if (m_events == NULL)
		return;
}

CTServConn::~CTServConn()
{
	/* 释放事件资源 */
	if (m_events)
	{
		free(m_events);
		m_events = NULL;
	}

	if (m_fd > 0)
	{
		close(m_fd);
		m_fd = -1;
	}
}

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
int CTServConn::Handle()
{
	if (m_fd == -1 || m_events == NULL || m_tserv == NULL)
		return -1;

	/* 等待epoll事件 */
	int nfds = epoll_wait(m_fd, m_events, m_max_client, 100);
	for (int i=0; i<nfds; i++)
	{
		if (m_events[i].events & EPOLLOUT)
		{
			LOG_TXT_DEBUG("EPOLLOUT: %d", m_events[i].data.fd);

			/* 处理发送请求 */
			m_tserv->HandleOut(m_events[i].data.fd);
		}
		else if (m_events[i].events & EPOLLIN)
		{
			LOG_TXT_DEBUG("EPOLLIN: %d", m_events[i].data.fd);

			/* 处理接收请求 */
			m_tserv->HandleIn(m_events[i].data.fd);
		}
		else if (m_events[i].events & (EPOLLERR | EPOLLHUP))
		{
			LOG_TXT_DEBUG("EPOLLHUP: %d", m_events[i].data.fd);

			/* 处理错误请求 */
			m_tserv->HandleErr(m_events[i].data.fd);
		}
	}

	return 0;
}

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
int CTServConn::Add(int sock, int flag /* = EPOLLIN |EPOLLOUT |EPOLLERR | EPOLLHUP */)
{
	if (m_fd == -1)
		return -1;

	struct epoll_event ev;

	ev.data.fd = sock;
	ev.events = flag;

	int ret = epoll_ctl(m_fd, EPOLL_CTL_ADD, sock, &ev);
	if (ret == -1)
	{
		LOG_TXT_ERR("error in epoll_ctl.[%d:%s]", sock, strerror(errno));
		return -1;
	}

	return 0;
}

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
int CTServConn::Mod(int sock, int flag /* = EPOLLIN |EPOLLOUT|EPOLLERR | EPOLLHUP */)
{
	if (m_fd == -1)
		return -1;

	struct epoll_event ev;

	ev.data.fd = sock;
	ev.events = flag;

	int ret = epoll_ctl(m_fd, EPOLL_CTL_MOD, sock, &ev);
	if (ret == -1)
	{
		LOG_TXT_ERR("error in epoll_ctl.[%d:%s]", sock, strerror(errno));
		return -1;
	}

	return 0;
}

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
int CTServConn::Remove(int sock)
{
	if (m_fd == -1)
		return -1;

	struct epoll_event ev;

	ev.data.fd = sock;
	ev.events = 0;

	int ret = epoll_ctl(m_fd, EPOLL_CTL_DEL, sock, &ev);
	if (ret == -1)
	{
		LOG_TXT_ERR("error in epoll_ctl.[%d:%s]", sock, strerror(errno));
		return -1;
	}

	return 0;
}

CTServTimeout::CTServTimeout(CTServFrm * tserv, int num, int timeout /* = -1 */) :
CRealTime(num, timeout), m_tserv(tserv)
{
}

CTServTimeout::~CTServTimeout()
{
}

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
int CTServTimeout::HandleTimeout(size_t key)
{
	int sock = (int)key;

	m_tserv->HandleTimeout(sock);

	return 0;
}

CTServFrm::CTServFrm(const char * name, const char * conf) :
	CTFrm(name, conf), m_conf(conf), m_cltnum(0), m_reqnum(0)
{
    m_serv = NULL;
}

CTServFrm::~CTServFrm()
{
    if (m_serv)
    {
        delete m_serv;
        m_serv = NULL;
    }
}

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
int CTServFrm::SetProcName(int argc, char * argv[])
{
	char * start = argv[0];
	char * end = argv[argc-1] + strlen(argv[argc-1]) + 1;

	/* 确定数据长度 */
	int len = strlen(m_conf.m_name);

	for (int i=1; i<argc; i++)
	{
		len += strlen(argv[i]);
	}

	/* 确保环境变量 */
	if ((end - start) < len)
	{
		for (int i=0; environ[i]; i++)
		{
			environ[i] = strdup(environ[i]);
		}
	}

	/* 确保其他参数 */
	for (int i=1; i<argc; i++)
	{
		char * p = argv[i];
		argv[i] = strdup(argv[i]);
		memset(p, 0, strlen(argv[i]));
	}

	/* 修改进程名 */
	memset(argv[0], 0, strlen(argv[0]));
	strcpy(argv[0], m_conf.m_name);

	return 0;
}

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
int CTServFrm::InitConf(const char * conf)
{
	if (m_conf.Init())
	{
		return -1;
	}

	LOG_TXT_NOTICE("InitConf ok.");

	return 0;
}

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
int CTServFrm::InitFrame()
{
	/* 初始化日志接口 */
	LOG_INIT(m_conf.m_log_path,
			 m_conf.m_log_mod,
			 m_conf.m_log_size,
			 m_conf.m_log_cnt,
			 m_conf.m_log_level,
			 m_conf.m_log_filter);

	/* 配置参数写日志 */
	m_conf.Log();

	/* 初始化缓冲池 */
	m_bufpool = new CTServBufferPool(m_conf.m_bufnum, m_conf.m_buflen);
	if (m_bufpool == NULL)
	{
		LOG_TXT_ERR("fail to create buf pool.");
		return -1;
	}

	LOG_TXT_NOTICE("buffer pool ok.");

	/* 初始化请求/响应对象表 */
	m_reqbuf = new CHash(m_conf.m_bufnum);
	if (m_reqbuf == NULL)
	{
		LOG_TXT_ERR("fail to create reqbuf(CHash).");
		return -1;
	}

	LOG_TXT_NOTICE("reqbuf(CHash) ok.");

	m_rspbuf = new CHash(m_conf.m_bufnum);
	if (m_reqbuf == NULL)
	{
		LOG_TXT_ERR("fail to create rspbuf(CHash).");
		return -1;
	}

	LOG_TXT_NOTICE("rspbuf(CHash) ok.");

	/* 初始化正在进行业务处理的句柄表 */
	m_inuse = new CHash(m_conf.m_bufnum);
	if (m_inuse == NULL)
	{
		LOG_TXT_ERR("fail to create inuse(CHash).");
		return -1;
	}

	LOG_TXT_NOTICE("inuse(CHash) ok.");

	/* 初始化连接处理对象 */
	m_epoller = new CTServConn(this, m_conf.m_maxclt);
	if (m_epoller == NULL)
	{
		LOG_TXT_ERR("fail to create epoll.");
		return -1;
	}

	LOG_TXT_NOTICE("tserv ok.");

	/* 初始化模块信息 */
	for (int i=0; i<m_conf.m_mod_num; i++)
	{
		recTServModule * module = (recTServModule *)&m_conf.m_module[i];

		/* 加载动态库 */
		module->modId = dlopen(module->modFile, RTLD_LAZY);
		if (module->modId == NULL)
		{
			LOG_TXT_ERR("fail to load %s[%s].", module->modFile, dlerror());
			return -1;
		}

		/* 获取函数地址 */
		char * error = NULL;
		dlerror();
		if (strlen(module->modInit)) /* 初始化 */
		{
			module->init = (nTServModInit)dlsym(module->modId, module->modInit);
			if ((error = dlerror()) != NULL)
			{
				LOG_TXT_ERR("fail to get [%s].[%s][%s] address.", module->modFile, module->modInit, error);
				return -1;
			}

			/* 初始化接口 */
			if (module->init(module->modConf))
			{
				LOG_TXT_ERR("fail to initialize module [%s]", module->modFile);
				return -1;
			}
		}
		else
		{
			module->init = NULL;
		}

		if (strlen(module->modDispatch)) /* 业务处理 */
		{
			module->dispatch = (nTServModDispatch)dlsym(module->modId, module->modDispatch);
			if ((error = dlerror()) != NULL)
			{
				LOG_TXT_ERR("fail to get [%s].[%s][%s] address.", module->modFile, module->modDispatch, error);
				return -1;
			}
		}
		else
		{
			module->dispatch = NULL;
		}

		if (strlen(module->modPreClose)) /* 退出预处理 */
		{
			module->preclose = (nTServModPreClose)dlsym(module->modId, module->modPreClose);
			if ((error = dlerror()) != NULL)
			{
				LOG_TXT_ERR("fail to get [%s].[%s][%s] address.", module->modFile, module->modPreClose, error);
				return -1;
			}
		}
		else
		{
			module->preclose = NULL;
		}

		if (strlen(module->modDestroy)) /* 资源释放 */
		{
			module->destroy = (nTServModDestroy)dlsym(module->modId, module->modDestroy);
			if ((error = dlerror()) != NULL)
			{
				LOG_TXT_ERR("fail to get [%s].[%s][%s] address.", module->modFile, module->modDestroy, error);
				return -1;
			}
		}
		else
		{
			module->destroy = NULL;
		}
	}

	LOG_TXT_NOTICE("module install.");

	/* 创建业务处理线程池 */
    m_pool = new CTSBizThreadPool(this, m_conf.m_tnum);
    if (m_pool == NULL)
    {
        LOG_TXT_ERR("Fail to create Thread Pool...");
        return -1;
    }

	LOG_TXT_NOTICE("CTSBizThreadPool ok.");

	/* 创建超时对象;加大超时节点的数目，避免因线程调度引起的漏删句柄 */
	m_timeout = new CTServTimeout(this, m_conf.m_maxclt*2, m_conf.m_timeout);
	if (m_timeout == NULL)
	{
        LOG_TXT_ERR("Fail to create CTServTimeout...");
        return -1;
	}

	/* 设置全局变量指针 */
	g_tservFrm = this;

	LOG_TXT_NOTICE("InitFrame ok.");

	return 0;
}

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
int CTServFrm::ReleaseFrame()
{
	/* 唤醒阻塞线程，预处理 */
	for (int i=0; i<m_conf.m_mod_num; i++)
	{
		recTServModule * module = (recTServModule *)&m_conf.m_module[i];

		if (module->modId)
		{
			/* 预处理 */
			if (module->preclose)
			{
				module->preclose();
			}
		}
	}

	LOG_TXT_NOTICE("module close.");

	/* 释放线程池，关闭线程 */
    if (m_pool)
    {
		m_pool->Stop();

        delete m_pool;
        m_pool = NULL;
    }

	LOG_TXT_NOTICE("biz-thread close.");

	/* 释放超时对象 */
    if (m_timeout)
    {
        delete m_timeout;
        m_timeout = NULL;
    }

	LOG_TXT_NOTICE("timeout close.");

	/* 卸掉模块信息 */
	for (int i=0; i<m_conf.m_mod_num; i++)
	{
		recTServModule * module = (recTServModule *)&m_conf.m_module[i];

		if (module->modId)
		{
			/* 释放模块资源 */
			if (module->destroy)
			{
				module->destroy();
			}

			dlclose(module->modId);
		}
	}

	LOG_TXT_NOTICE("module uninstall.");

	/* 释放epoll资源 */
	if (m_epoller)
	{
		delete m_epoller;
		m_epoller = NULL;
	}

	/* 释放INUSE表 */
	if (m_inuse)
	{
		delete m_inuse;
		m_inuse = NULL;
	}

	/* 释放请求/响应表 */
	if (m_rspbuf)
	{
		delete m_rspbuf;
		m_rspbuf = NULL;
	}

	if (m_reqbuf)
	{
		delete m_reqbuf;
		m_reqbuf = NULL;
	}

	/* 释放缓冲资源 */
	if (m_bufpool)
	{
		delete m_bufpool;
		m_bufpool = NULL;
	}

	LOG_TXT_NOTICE("ReleaseFrame ok.");

	return 0;
}

/*
 * Function: HandleThread
 * Desc: 线程业务处理
 * In:
 *      int    ibreak   中断服务标志
 * Out:
 *      int    ibreak   中断服务标志
 * Return code:
 *      -1      出错
 *       0      成功
 */
int CTServFrm::HandleThread(int& ibreak)
{
    /* 创建TCP Server */
    if (m_serv)
    {
		LOG_TXT_ERR("TCP Server exist...");
        ibreak = 1;
		return -1;
    }

    m_serv = new CSockAcceptor(m_conf.m_ip, m_conf.m_port, m_conf.m_backlog);
    if (m_serv == NULL)
    {
        LOG_TXT_ERR("Fail to create TCP Server...");
        ibreak = 1;
        return -1;
    }

	/* 设置为非阻塞句柄 */
	m_serv->SetNonBlockOption();

	/* 放入EPOLL队列中，获取IN事件，等待接收连接 */
	m_epoller->Add(m_serv->GetFD(), EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP);

    while (IsRunning())
    {
		/* EPOLL侦听数据 */
		m_epoller->Handle();
	}

	/* 从EPOLL队列中剔除 */
	m_epoller->Remove(m_serv->GetFD());

	/* 关闭句柄 */
	m_serv->Close();

	ibreak = 1;

	/* 退出线程 */
	LOG_TXT_INFO("thread exit.");

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
int CTServFrm::OnStop()
{
	LOG_TXT_NOTICE("close listen-port.");

	/* 唤醒TIMEOUT */
	m_timeout->WakeUp();

	LOG_TXT_NOTICE("timeout wake up.");

	/* 唤醒BufferPool */
	m_bufpool->WakeUpAll();

	LOG_TXT_NOTICE("buffer pool wake up.");

    return 0;
}

/*
 * Function: HandleClient
 * Desc: 处理客户端信息
 * In:
 *      const char *   buf     数据缓冲
 *      int            buflen  数据长度
 *      CTSBizThread * thread  线程
 * Out:
 *      none
 * Return code:
 *      0     -   成功
 *      <0    -   失败
 */
int CTServFrm::HandleClient(const char * buf, int buflen)
{
	recTServProtoPck * prTServProtoPck = (recTServProtoPck *)buf;
	recTServModule * module = m_conf.GetModule(prTServProtoPck->head.cmd);
	if (module == NULL)
	{
		LOG_TXT_ERR("fail to get module.");
		return -1;
	}

	if (module->dispatch == NULL)
	{
		LOG_TXT_ERR("[%d - %s] dispatch is NULL", prTServProtoPck->head.cmd, module->name);
		return -1;
	}

	/* 业务处理 */
	int ret = module->dispatch(buf,
							   buflen,
							   CTServFrm::nTServWrite);
	if (ret < 0)
	{
		LOG_TXT_ERR("fail in [%s]nTServModDispatch.", module->name);
		return -1;
	}

	return 0;
}

/*
 * Function: nTServWrite
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
int CTServFrm::nTServWrite(const char * buf, int buflen)
{
	/* 取线程句柄 */
	pthread_t h = pthread_self();
	CTSBizThread * thread = (CTSBizThread *)((g_tservFrm->GetTSBizThreadPool())->find(h));
	if (thread == NULL)
	{
		LOG_TXT_ERR("fail to get thread.");
		return -1;
	}

	/* 获取线程对应缓冲对象 */
	CTServBuffer * buff = thread->GetBuffer();
	if (buff == NULL)
	{
		LOG_TXT_ERR("buffer invalid.");
		return -1;
	}

	/* 发送数据 */
	int ret = buff->Write(buf, buflen);
	if (ret < 0)
	{
		/* 发送失败 */
		buff->SetStatus(CTServBuffer::E_UNUSED); /* 无用的句柄 */
		LOG_TXT_ERR("fail to send data.[%d:%d][%s]", buff->GetSock(), ret, strerror(errno));
		return -1;
	}

	return 0;
}

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
int CTServFrm::AcceptConnect()
{
	while (1)
	{
		/* 接收链接 */
		int sock = m_serv->Accept();
		if (sock < 0)
		{
			if (errno == EAGAIN)
				/* 全部接收完成 */
				break;

			if (errno == EINTR)
				/* 被信号中断，还可接收 */
				continue;

			/* 接收链接失败 */
			LOG_TXT_ERR("fail to accept connect.");

			return -1;
		}

		/* 句柄是否被复用，复用关闭链接，等待客户端重连 */
		if (m_inuse->Get(sock))
		{
			LOG_TXT_NOTICE("socket [%d] in use.", sock);
			close(sock);
			return -1;
		}

		/* 设置为非阻塞 */
		CBaseSocket::SetNonBlockOption(sock);

		/* 取业务处理对象，获取IN事件，接收数据 */
		if (m_epoller->Add(sock, EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP))
		{
			LOG_TXT_ERR("fail to set [%d] in epoll...", sock);
			/* 关闭连接 */
			close(sock);
			return -1;
		}

		/* 记载操作时间 */
		if (m_timeout->Use((size_t)sock) < 0)
		{
			LOG_TXT_ERR("fail to use [%d] in timeout...", sock);
			/* 关闭连接 */
			close(sock);
			return -1;
		}

		/* 连接客户数+1 */
		m_cltnum++;

		LOG_TXT_INFO("accept a connection[%d].", sock);
	}

	return 0;
}

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
int CTServFrm::HandleIn(int sock)
{
	if (sock == m_serv->GetFD())
	{
		/* 侦听端口接收链接 */
		return AcceptConnect();
	}

	/* 从请求表中查找对应的缓冲处理对象 */
	CTServBuffer * buff = (CTServBuffer *)m_reqbuf->Remove((size_t)sock);
	if (buff == NULL)
	{
		/* 未找到获取新对象 */
		buff = m_bufpool->get(sock);
		if (buff == NULL)
		{
			LOG_TXT_ERR("fail to get buff.[%d]", sock);
			HandleErr(sock); /* 移除关闭SOCKET句柄 */
			return -1;
		}
	}

	/* 接收数据 */
	int ret = buff->Recv();
	if (ret < 0)
	{
		LOG_TXT_INFO("fail to receive data.[%d]", sock);
		/* 接收数据失败 */
		HandleErr(sock, buff);
		return -1;
	}

	if (ret > 0)
	{
		/* 未接收到完整数据，需要接着接收,放入接收表 */
		m_reqbuf->Add((size_t)sock, buff);
		return 0;
	}

	/* 数据完整，可进行业务处理 */
	CTSBizThread * bizThread = m_pool->get(buff);
	if (bizThread == NULL)
	{
		LOG_TXT_ERR("fail to get bizthread...[%d]", sock);
		/* 关闭连接 */
		HandleErr(sock, buff); /* 移除关闭句柄 */
		return -1;
	}

	/* 记载操作时间 */
	m_timeout->Use((size_t)sock);

	/* 统计数目 */
	m_reqnum++;

	return 0;
}

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
int CTServFrm::HandleOut(int sock)
{
	/* 找到对应的缓冲处理对象 */
	CTServBuffer * buff = (CTServBuffer *)m_rspbuf->Remove((size_t)sock);
	if (buff == NULL)
	{
		LOG_TXT_ERR("fail to find buff.[%d]", sock);
		HandleErr(sock); /* 移除关闭句柄 */
		return -1;
	}

	/* 发送数据 */
	int ret = buff->Send();
	if (ret < 0)
	{
		LOG_TXT_ERR("fail to send data.[%d]", sock);
		HandleErr(sock, buff); /* 移除关闭句柄 */
		return -1;
	}

	if (ret == 0)
	{
		LOG_TXT_DEBUG("all are sent.[%d][left:%d]", sock, buff->GetResponseLeft());

		/* 设置BUFF为空闲状态 */
		buff->SetStatus(CTServBuffer::E_IDLE); /* 无用的句柄 */

		/* 业务处理完成 */
		EndProc(buff);

		/* 修改句柄侦听事件，获取IN事件，等待接收数据 */
		m_epoller->Mod(sock, EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP);
	}
	else
	{
		LOG_TXT_DEBUG("someone not sent.[%d]", sock);

		/* 重新放入响应表，等待下次发送 */
		m_rspbuf->Add((size_t)sock, buff);
	}

	return 0;
}

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
int CTServFrm::HandleErr(int sock, CTServBuffer * buff /* = NULL */)
{
	if (sock == m_serv->GetFD())
	{
		/* 从epoll中移除 */
		m_epoller->Remove(sock);

		/* 关闭链接缓冲 */
		close(sock);

		LOG_TXT_INFO("close server[%d].", sock);

		return 0;
	}

	/* 确认句柄是否还在使用中 */
	if (m_inuse->Get((size_t)sock) && (buff == NULL))
	{
		LOG_TXT_INFO("Biz Thread is alive now.[%d]", sock);
		return 0;
	}

	/* 确认BUFF状态 */
	buff = ((buff == NULL) ? (CTServBuffer *)m_reqbuf->Remove((size_t)sock) : buff);
	buff = ((buff == NULL) ? (CTServBuffer *)m_rspbuf->Remove((size_t)sock) : buff);

	if (buff)
	{
		/* 设置BUFF为空闲状态 */
		buff->SetStatus(CTServBuffer::E_IDLE); /* 无用的句柄 */

		/* 业务处理完成 */
		EndProc(buff);
	}

	/* 超时队列中清除 */
	m_timeout->UnUse((size_t)sock);

	/* 从epoll中移除 */
	if (m_epoller->Remove(sock))
		return -1;

	/* 关闭链接缓冲 */
	close(sock);

	/* 连接客户数-1 */
	m_cltnum--;

	LOG_TXT_INFO("close a connection[%d].", sock);

	return 0;
}

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
int CTServFrm::ReloadConf(const char *conf)
{
	/* 记载系统一些统计日志 */
	LOG_TXT_NOTICE("SYSTEM INFO:[THREAD: %d %d][BUFF: %d %d][CONN: %u][INUSE: %d][RECV: %llu]",
		m_pool->get_num(), m_pool->get_idle_num(),
		m_bufpool->get_num(), m_bufpool->get_idle_num(), m_cltnum, m_inuse->GetNodeNum(), m_reqnum);

	LOG_TXT_NOTICE("TIMEOUT INFO:[FREE: %d][NEW: %d][TRASH: %d][TIME: %d][INUSE: %d]",
		m_timeout->GetFreeNum(), m_timeout->GetNewNum(), m_timeout->GetTrashNum(),
		m_timeout->GetTimeNum(), m_timeout->GetUseNum());

	return 0;
}

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
int CTServFrm::HandleOut(CTServBuffer * buff)
{
	LOG_TXT_DEBUG("unsent data.HandleOut: %d", buff->GetSock());

	/* 将buff放入响应队列 */
	m_rspbuf->Add((size_t)buff->GetSock(), buff);

	/* 修改句柄侦听事件，获取OUT事件，等待发送数据 */
	m_epoller->Mod(buff->GetSock(), EPOLLOUT | EPOLLET | EPOLLERR | EPOLLHUP);

	return 0;
}

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
int CTServFrm::PreProc(CTServBuffer * buff)
{
	/* 将句柄放入处理表中 */
	m_inuse->Add((size_t)(buff->GetSock()), buff);

	return 0;
}

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
int CTServFrm::EndProc(CTServBuffer * buff)
{
	int sock = buff->GetSock();
	CTServBuffer::enuBuffStatus status = buff->GetStatus();

	LOG_TXT_DEBUG("end of the process.[sock:%d status:%d]", sock, status);

	/* 检查buff状态 */
	switch (status)
	{
	case CTServBuffer::E_IDLE:
		{
			/* 从处理表中移出 */
			m_inuse->Remove((size_t)(buff->GetSock()));

			/* BUFF处于空闲状态，业务已完成，可以回收 */
			m_bufpool->release(buff);
		}
		break;
	case CTServBuffer::E_INUSE:
		{
			/* BUFF中存在数据，需要发送出去,业务处理未完成 */
			HandleOut(buff);
			return 0;
		}
		break;
	case CTServBuffer::E_UNUSED:
		{
			/* 业务已完成，BUFF中数据已坏，需要关闭连接 */
			HandleErr(sock, buff);
		}
		break;
	default:
		LOG_TXT_ERR("the status of buffer is invalid.[%d-%d]", buff->GetSock(), buff->GetStatus());
		return -1;
	}

	return 0;
}

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
int CTServFrm::HandleTimeout(int sock)
{
	/* 关闭句柄，由EPOLL来收拾现场 */
	shutdown(sock, SHUT_RDWR);

	LOG_TXT_INFO("timeout. close a connection[%d].", sock);

	return 0;
}
