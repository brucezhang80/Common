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
    Start(); /* �����߳� */
}

CTSBizThread::~CTSBizThread()
{
}

/*
 * Function: HandleThread
 * Desc: �߳�ҵ����
 * In:
 *      int    ibreak   �жϷ����־
 * Out:
 *      int    ibreak   �жϷ����־
 * Return code:
 *      -1      ����
 *       0      �ɹ�
 */
int CTSBizThread::HandleThread(int& ibreak)
{
	if (m_buff == NULL)
	{
		return -1;
	}

	/* �������� */
	m_tserv->HandleClient(m_buff->GetRequest(), m_buff->GetRequestLength());

	LOG_TXT_INFO("handle client over;");

	/* ������� */
	m_tserv->EndProc(m_buff);

	LOG_TXT_INFO("end proc over;");

	/* �ͷŶ������ڿ���״̬���ȴ��µ����� */
    m_pool->release(this);

    LOG_TXT_INFO("release pool over.");

    return 0;
}

/*
 * Function: Attach
 * Desc: �󶨾��
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTSBizThread::Attach(CTServBuffer * buff)
{
	m_buff = buff;

    return 0;
}

/*
 * Function: Detach
 * Desc: ж�ؾ��
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTSBizThread::Detach()
{
    m_buff = NULL;

    return 0;
}

/*
 * Function: Resume
 * Desc: �����߳�
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTSBizThread::Resume(CTServBuffer * buff)
{
    /* �󶨾�� */
    Attach(buff);

	/* ����Ԥ���� */
	m_tserv->PreProc(buff);

    /* �����߳� */
    WakeUp();

    return 0;
}

/*
 * Function: Pause
 * Desc: �����߳�
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
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
 * Desc: ��������
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      ��NULL  -   �ɹ�
 *      NULL    -   ʧ��
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
        /* �����߳� */
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
    /* ��ͣ�߳� */
    elem->Pause();

    return CObjPool<CTSBizThread>::release(elem);
}

/******************************************************************************
 * FUNCTION:  CObjPool.get
 * DESCRIPTION:  ��ȡ�������е��̶߳���
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
	/* �Ƿ��˳� */
	if (m_isstop)
		return NULL;

	return (CTSBizThread *)(m_thread_list.Get((unsigned int)h));
}

/*
 * Function: Stop
 * Desc: ֹͣ���д����߳�
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTSBizThreadPool::Stop()
{
	/* �����˳���־ */
	m_isstop = 1;

	/* ��ֹ�����߳� */
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
 * Desc: ����TCP���
 * In:
 *      int  sock   TCP���
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTServBuffer::Attach(int sock)
{
	m_sock.Attach(sock, SOCK_STREAM);

	/* ������� */
	m_reqlen = m_reqleft = m_rspleft = 0;
	m_status = E_IDLE;

	return 0;
}

/*
 * Function: Detach
 * Desc: �ͷž��
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTServBuffer::Detach()
{
	m_sock.Dettach();

	return 0;
}

/*
 * Function: Recv
 * Desc: ��������
 * In:
 *      char *     buf  ��Ӧ����
 *      int        len  ���ݳ���
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTServBuffer::Recv(char * buf, int len)
{
	int left = len;
	char * ptr = buf;

	/* ���Ϊ������ */
	while (left > 0)
	{
		/* �������� */
		int ret = m_sock.Recv(ptr, left);
		if (ret <= 0)
		{
			if (ret == -1 && errno == EAGAIN)
				/* �������� */
				break;

			if (ret == -1 && errno == EINTR)
				/* ���ź��жϣ��������� */
				continue;

			return -1;
		}

		left -= ret;
		ptr += ret;
	}

	/* ����ʵ�ʶ�ȡ������ */
	return (len - left);
}

/*
 * Function: Recv
 * Desc: ��������
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTServBuffer::Recv()
{
	if (m_request == NULL || !m_sock.IsValid())
		return -1;

	/* �������� */
	char * ptr = m_request + m_reqlen - m_reqleft;
	int left = m_buflen - (m_reqlen - m_reqleft);
	int ret = Recv(ptr, left);
	if (ret <= 0 || ret < (int)sizeof(unsigned int)) /* 4���ֽ�Ҳû�յ���Ϊ�������� */
	{
		return -1;
	}

	/* ������� */
	int len = m_reqlen - m_reqleft + ret;

	/* ȡ���ݳ��� */
	memcpy(&m_reqlen, m_request, sizeof(unsigned int));

	/* ���ڴ������� */
	if (len > m_reqlen)
	{
		LOG_TXT_ERR("invalid request.[%d]", m_sock.GetFD());
		return -1;
	}

	m_reqleft = m_reqlen - len;

	if (m_reqleft > 0)
	{
		/* ���������� */
		return 1;
	}

	LOG_BIN_DEBUG(m_request, m_reqlen);

	return 0;
}

/*
 * Function: Send
 * Desc: ��������
 * In:
 *      const char *   buf  ��Ӧ����
 *      int            len  ���ݳ���
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTServBuffer::Send(const char * buf, int len)
{
	if (m_response == NULL || !m_sock.IsValid())
		return -1;

	/* �������� */
	int left = len;
	char * ptr = (char *)buf;

	while (left > 0)
	{
		int ret = m_sock.Send(ptr, left);
		if (ret <= 0)
		{
			if (errno == EAGAIN)
				/* �ӻ����� */
				break;

			if (errno == EINTR)
				/* ���ź��жϣ����ɷ� */
				continue;

			return -1;
		}

		left -= ret;
		ptr += ret;
	}

	LOG_TXT_DEBUG("Send:len: %d left: %d", len, left);

	LOG_BIN_DEBUG(buf, len - left);

	/* ����ʵ�ʷ������ݴ�С */
	return (len - left);
}

/*
 * Function: Send
 * Desc: ����ʣ������
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTServBuffer::Send()
{
	if (m_response == NULL || !m_sock.IsValid())
		return -1;

	/* �������� */
	int ret = Send(m_response, m_rspleft);
	if (ret < 0)
	{
		return -1;
	}

	/* ȷ���Ƿ񻹴���ʣ������ */
	m_rspleft -= ret;
	if (m_rspleft > 0)
	{
		/* �����ڴ���������,����ʣ������ */
		char * ptr = m_response + ret;
		memcpy(m_response, ptr, m_rspleft);
		m_status = E_INUSE; /* �������� */
		return 1;
	}

	m_status = E_IDLE; /* ������ȫ����� */

	return 0;
}

/*
 * Function: Write
 * Desc: ��������
 * In:
 *      const char *   buf  ��Ӧ����
 *      int            len  ���ݳ���
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTServBuffer::Write(const char * buf, int len)
{
	if (m_response == NULL || m_status == E_UNUSED)
		return -1;

	/* ����Ƿ����δ������� */
	if (m_rspleft > 0)
	{
		/* �ȷ����������� */
		int ret = Send();
		if (ret < 0)
			return -1;

		if (ret > 0)
		{
			/* ����δ�������� */
			char * ptr = m_response + m_rspleft;
			if ((len + m_rspleft) >= m_buflen)
				return -2;
			memcpy(ptr, buf, len);
			m_rspleft += len;
			m_status = E_INUSE; /* �������� */
			return 1;
		}
	}

	/* �������� */
	int ret = Send(buf, len);
	if (ret < 0)
		return -1;

	/* ȷ���Ƿ����ʣ�� */
	m_rspleft = len - ret;
	if (m_rspleft > 0)
	{
		/* ����ʣ������,���ݹ� */
		char * ptr = (char *)buf + ret;
		memcpy(m_response, ptr, m_rspleft);
		m_status = E_INUSE; /* �������� */
		return 1;
	}

	m_status = E_IDLE; /* ������ȫ����� */

	return 0;
}

CTServBufferPool::CTServBufferPool(int num, int buflen) :
CObjPool<CTServBuffer>(num), m_buflen(buflen)
{
    /* ��ʼ������� */
    init();
}

CTServBufferPool::~CTServBufferPool()
{
}

/*
 * Function: newobject
 * Desc: ��������
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      ��NULL  -   �ɹ�
 *      NULL    -   ʧ��
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

	/* ��TCP���� */
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
	/* �ͷ�TCP��� */
	elem->Detach();

    return CObjPool<CTServBuffer>::release(elem);
}

CTServConn::CTServConn(CTServFrm * tserv, int max_client) :
m_tserv(tserv), m_fd(-1), m_max_client(max_client), m_events(NULL)
{
	/* ����epoll */
	int size =  max_client < 1024 ? 1024 : max_client;
	m_fd = epoll_create(size);
	if (m_fd == -1)
		return;

	/* ȷ��������¼��ռ� */
	m_events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * (size + 1024));
	if (m_events == NULL)
		return;
}

CTServConn::~CTServConn()
{
	/* �ͷ��¼���Դ */
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
 * Desc: �߳�ҵ����
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CTServConn::Handle()
{
	if (m_fd == -1 || m_events == NULL || m_tserv == NULL)
		return -1;

	/* �ȴ�epoll�¼� */
	int nfds = epoll_wait(m_fd, m_events, m_max_client, 100);
	for (int i=0; i<nfds; i++)
	{
		if (m_events[i].events & EPOLLOUT)
		{
			LOG_TXT_DEBUG("EPOLLOUT: %d", m_events[i].data.fd);

			/* ���������� */
			m_tserv->HandleOut(m_events[i].data.fd);
		}
		else if (m_events[i].events & EPOLLIN)
		{
			LOG_TXT_DEBUG("EPOLLIN: %d", m_events[i].data.fd);

			/* ����������� */
			m_tserv->HandleIn(m_events[i].data.fd);
		}
		else if (m_events[i].events & (EPOLLERR | EPOLLHUP))
		{
			LOG_TXT_DEBUG("EPOLLHUP: %d", m_events[i].data.fd);

			/* ����������� */
			m_tserv->HandleErr(m_events[i].data.fd);
		}
	}

	return 0;
}

/*
 * Function: Add
 * Desc: ���Ӵ�����TCP���
 * In:
 *      int  sock   TCP���
 *      int  flag   �¼�MASK
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
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
 * Desc: �޸Ĵ�����TCP���
 * In:
 *      int  sock   TCP���
 *      int  flag   �¼�MASK
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
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
 * Desc: �Ƴ�������TCP���
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
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
 * Desc: ��ʱ����
 * In:
 *      size_t key
 * Out:
 *      none
 * Return code:
 *      0  -   �ɹ�
 *      -1 -   ʧ��
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
 * Desc: ���ý�������
 * In:
 *      int    argc    ������Ŀ
 *      char * argv[]  �������
 * Out:
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTServFrm::SetProcName(int argc, char * argv[])
{
	char * start = argv[0];
	char * end = argv[argc-1] + strlen(argv[argc-1]) + 1;

	/* ȷ�����ݳ��� */
	int len = strlen(m_conf.m_name);

	for (int i=1; i<argc; i++)
	{
		len += strlen(argv[i]);
	}

	/* ȷ���������� */
	if ((end - start) < len)
	{
		for (int i=0; environ[i]; i++)
		{
			environ[i] = strdup(environ[i]);
		}
	}

	/* ȷ���������� */
	for (int i=1; i<argc; i++)
	{
		char * p = argv[i];
		argv[i] = strdup(argv[i]);
		memset(p, 0, strlen(argv[i]));
	}

	/* �޸Ľ����� */
	memset(argv[0], 0, strlen(argv[0]));
	strcpy(argv[0], m_conf.m_name);

	return 0;
}

/*
 * Function: InitConf
 * Desc: ��ʼ�����ò���
 * In:
 *     const char *   conf  �����ļ���
 * Out:
 *     none
 * Return code:
 *     -1                   ����
 *      0                   �ɹ�
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
 * Desc: ϵͳ��ʼ��
 * In:
 *     none
 * Out:
 *     none
 * Return code:
 *     -1                   ����
 *      0                   �ɹ�
 */
int CTServFrm::InitFrame()
{
	/* ��ʼ����־�ӿ� */
	LOG_INIT(m_conf.m_log_path,
			 m_conf.m_log_mod,
			 m_conf.m_log_size,
			 m_conf.m_log_cnt,
			 m_conf.m_log_level,
			 m_conf.m_log_filter);

	/* ���ò���д��־ */
	m_conf.Log();

	/* ��ʼ������� */
	m_bufpool = new CTServBufferPool(m_conf.m_bufnum, m_conf.m_buflen);
	if (m_bufpool == NULL)
	{
		LOG_TXT_ERR("fail to create buf pool.");
		return -1;
	}

	LOG_TXT_NOTICE("buffer pool ok.");

	/* ��ʼ������/��Ӧ����� */
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

	/* ��ʼ�����ڽ���ҵ����ľ���� */
	m_inuse = new CHash(m_conf.m_bufnum);
	if (m_inuse == NULL)
	{
		LOG_TXT_ERR("fail to create inuse(CHash).");
		return -1;
	}

	LOG_TXT_NOTICE("inuse(CHash) ok.");

	/* ��ʼ�����Ӵ������ */
	m_epoller = new CTServConn(this, m_conf.m_maxclt);
	if (m_epoller == NULL)
	{
		LOG_TXT_ERR("fail to create epoll.");
		return -1;
	}

	LOG_TXT_NOTICE("tserv ok.");

	/* ��ʼ��ģ����Ϣ */
	for (int i=0; i<m_conf.m_mod_num; i++)
	{
		recTServModule * module = (recTServModule *)&m_conf.m_module[i];

		/* ���ض�̬�� */
		module->modId = dlopen(module->modFile, RTLD_LAZY);
		if (module->modId == NULL)
		{
			LOG_TXT_ERR("fail to load %s[%s].", module->modFile, dlerror());
			return -1;
		}

		/* ��ȡ������ַ */
		char * error = NULL;
		dlerror();
		if (strlen(module->modInit)) /* ��ʼ�� */
		{
			module->init = (nTServModInit)dlsym(module->modId, module->modInit);
			if ((error = dlerror()) != NULL)
			{
				LOG_TXT_ERR("fail to get [%s].[%s][%s] address.", module->modFile, module->modInit, error);
				return -1;
			}

			/* ��ʼ���ӿ� */
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

		if (strlen(module->modDispatch)) /* ҵ���� */
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

		if (strlen(module->modPreClose)) /* �˳�Ԥ���� */
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

		if (strlen(module->modDestroy)) /* ��Դ�ͷ� */
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

	/* ����ҵ�����̳߳� */
    m_pool = new CTSBizThreadPool(this, m_conf.m_tnum);
    if (m_pool == NULL)
    {
        LOG_TXT_ERR("Fail to create Thread Pool...");
        return -1;
    }

	LOG_TXT_NOTICE("CTSBizThreadPool ok.");

	/* ������ʱ����;�Ӵ�ʱ�ڵ����Ŀ���������̵߳��������©ɾ��� */
	m_timeout = new CTServTimeout(this, m_conf.m_maxclt*2, m_conf.m_timeout);
	if (m_timeout == NULL)
	{
        LOG_TXT_ERR("Fail to create CTServTimeout...");
        return -1;
	}

	/* ����ȫ�ֱ���ָ�� */
	g_tservFrm = this;

	LOG_TXT_NOTICE("InitFrame ok.");

	return 0;
}

/*
 * Function: ReleaseFrame
 * Desc: ϵͳ��Դ����
 * In:
 *     none
 * Out:
 *     none
 * Return code:
 *     -1                   ����
 *      0                   �ɹ�
 */
int CTServFrm::ReleaseFrame()
{
	/* ���������̣߳�Ԥ���� */
	for (int i=0; i<m_conf.m_mod_num; i++)
	{
		recTServModule * module = (recTServModule *)&m_conf.m_module[i];

		if (module->modId)
		{
			/* Ԥ���� */
			if (module->preclose)
			{
				module->preclose();
			}
		}
	}

	LOG_TXT_NOTICE("module close.");

	/* �ͷ��̳߳أ��ر��߳� */
    if (m_pool)
    {
		m_pool->Stop();

        delete m_pool;
        m_pool = NULL;
    }

	LOG_TXT_NOTICE("biz-thread close.");

	/* �ͷų�ʱ���� */
    if (m_timeout)
    {
        delete m_timeout;
        m_timeout = NULL;
    }

	LOG_TXT_NOTICE("timeout close.");

	/* ж��ģ����Ϣ */
	for (int i=0; i<m_conf.m_mod_num; i++)
	{
		recTServModule * module = (recTServModule *)&m_conf.m_module[i];

		if (module->modId)
		{
			/* �ͷ�ģ����Դ */
			if (module->destroy)
			{
				module->destroy();
			}

			dlclose(module->modId);
		}
	}

	LOG_TXT_NOTICE("module uninstall.");

	/* �ͷ�epoll��Դ */
	if (m_epoller)
	{
		delete m_epoller;
		m_epoller = NULL;
	}

	/* �ͷ�INUSE�� */
	if (m_inuse)
	{
		delete m_inuse;
		m_inuse = NULL;
	}

	/* �ͷ�����/��Ӧ�� */
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

	/* �ͷŻ�����Դ */
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
 * Desc: �߳�ҵ����
 * In:
 *      int    ibreak   �жϷ����־
 * Out:
 *      int    ibreak   �жϷ����־
 * Return code:
 *      -1      ����
 *       0      �ɹ�
 */
int CTServFrm::HandleThread(int& ibreak)
{
    /* ����TCP Server */
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

	/* ����Ϊ��������� */
	m_serv->SetNonBlockOption();

	/* ����EPOLL�����У���ȡIN�¼����ȴ��������� */
	m_epoller->Add(m_serv->GetFD(), EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP);

    while (IsRunning())
    {
		/* EPOLL�������� */
		m_epoller->Handle();
	}

	/* ��EPOLL�������޳� */
	m_epoller->Remove(m_serv->GetFD());

	/* �رվ�� */
	m_serv->Close();

	ibreak = 1;

	/* �˳��߳� */
	LOG_TXT_INFO("thread exit.");

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
int CTServFrm::OnStop()
{
	LOG_TXT_NOTICE("close listen-port.");

	/* ����TIMEOUT */
	m_timeout->WakeUp();

	LOG_TXT_NOTICE("timeout wake up.");

	/* ����BufferPool */
	m_bufpool->WakeUpAll();

	LOG_TXT_NOTICE("buffer pool wake up.");

    return 0;
}

/*
 * Function: HandleClient
 * Desc: ����ͻ�����Ϣ
 * In:
 *      const char *   buf     ���ݻ���
 *      int            buflen  ���ݳ���
 *      CTSBizThread * thread  �߳�
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      <0    -   ʧ��
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

	/* ҵ���� */
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
 * Desc: ����ͻ�����Ϣ
 * In:
 *      const char *   buf     ���ݻ���
 *      int            buflen  ���ݳ���
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      <0    -   ʧ��
 */
int CTServFrm::nTServWrite(const char * buf, int buflen)
{
	/* ȡ�߳̾�� */
	pthread_t h = pthread_self();
	CTSBizThread * thread = (CTSBizThread *)((g_tservFrm->GetTSBizThreadPool())->find(h));
	if (thread == NULL)
	{
		LOG_TXT_ERR("fail to get thread.");
		return -1;
	}

	/* ��ȡ�̶߳�Ӧ������� */
	CTServBuffer * buff = thread->GetBuffer();
	if (buff == NULL)
	{
		LOG_TXT_ERR("buffer invalid.");
		return -1;
	}

	/* �������� */
	int ret = buff->Write(buf, buflen);
	if (ret < 0)
	{
		/* ����ʧ�� */
		buff->SetStatus(CTServBuffer::E_UNUSED); /* ���õľ�� */
		LOG_TXT_ERR("fail to send data.[%d:%d][%s]", buff->GetSock(), ret, strerror(errno));
		return -1;
	}

	return 0;
}

/*
 * Function: AcceptConnect
 * Desc: ���տͻ�������
 * In:
 *      none
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      -1    -   ʧ��
 */
int CTServFrm::AcceptConnect()
{
	while (1)
	{
		/* �������� */
		int sock = m_serv->Accept();
		if (sock < 0)
		{
			if (errno == EAGAIN)
				/* ȫ��������� */
				break;

			if (errno == EINTR)
				/* ���ź��жϣ����ɽ��� */
				continue;

			/* ��������ʧ�� */
			LOG_TXT_ERR("fail to accept connect.");

			return -1;
		}

		/* ����Ƿ񱻸��ã����ùر����ӣ��ȴ��ͻ������� */
		if (m_inuse->Get(sock))
		{
			LOG_TXT_NOTICE("socket [%d] in use.", sock);
			close(sock);
			return -1;
		}

		/* ����Ϊ������ */
		CBaseSocket::SetNonBlockOption(sock);

		/* ȡҵ������󣬻�ȡIN�¼����������� */
		if (m_epoller->Add(sock, EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP))
		{
			LOG_TXT_ERR("fail to set [%d] in epoll...", sock);
			/* �ر����� */
			close(sock);
			return -1;
		}

		/* ���ز���ʱ�� */
		if (m_timeout->Use((size_t)sock) < 0)
		{
			LOG_TXT_ERR("fail to use [%d] in timeout...", sock);
			/* �ر����� */
			close(sock);
			return -1;
		}

		/* ���ӿͻ���+1 */
		m_cltnum++;

		LOG_TXT_INFO("accept a connection[%d].", sock);
	}

	return 0;
}

/*
 * Function: HandleIn
 * Desc: ����ͻ��˽�������
 * In:
 *      int            sock  TCP���
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      -1    -   ʧ��
 */
int CTServFrm::HandleIn(int sock)
{
	if (sock == m_serv->GetFD())
	{
		/* �����˿ڽ������� */
		return AcceptConnect();
	}

	/* ��������в��Ҷ�Ӧ�Ļ��崦����� */
	CTServBuffer * buff = (CTServBuffer *)m_reqbuf->Remove((size_t)sock);
	if (buff == NULL)
	{
		/* δ�ҵ���ȡ�¶��� */
		buff = m_bufpool->get(sock);
		if (buff == NULL)
		{
			LOG_TXT_ERR("fail to get buff.[%d]", sock);
			HandleErr(sock); /* �Ƴ��ر�SOCKET��� */
			return -1;
		}
	}

	/* �������� */
	int ret = buff->Recv();
	if (ret < 0)
	{
		LOG_TXT_INFO("fail to receive data.[%d]", sock);
		/* ��������ʧ�� */
		HandleErr(sock, buff);
		return -1;
	}

	if (ret > 0)
	{
		/* δ���յ��������ݣ���Ҫ���Ž���,������ձ� */
		m_reqbuf->Add((size_t)sock, buff);
		return 0;
	}

	/* �����������ɽ���ҵ���� */
	CTSBizThread * bizThread = m_pool->get(buff);
	if (bizThread == NULL)
	{
		LOG_TXT_ERR("fail to get bizthread...[%d]", sock);
		/* �ر����� */
		HandleErr(sock, buff); /* �Ƴ��رվ�� */
		return -1;
	}

	/* ���ز���ʱ�� */
	m_timeout->Use((size_t)sock);

	/* ͳ����Ŀ */
	m_reqnum++;

	return 0;
}

/*
 * Function: HandleOut
 * Desc: ����ͻ������
 * In:
 *      int            sock  TCP���
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      -1    -   ʧ��
 */
int CTServFrm::HandleOut(int sock)
{
	/* �ҵ���Ӧ�Ļ��崦����� */
	CTServBuffer * buff = (CTServBuffer *)m_rspbuf->Remove((size_t)sock);
	if (buff == NULL)
	{
		LOG_TXT_ERR("fail to find buff.[%d]", sock);
		HandleErr(sock); /* �Ƴ��رվ�� */
		return -1;
	}

	/* �������� */
	int ret = buff->Send();
	if (ret < 0)
	{
		LOG_TXT_ERR("fail to send data.[%d]", sock);
		HandleErr(sock, buff); /* �Ƴ��رվ�� */
		return -1;
	}

	if (ret == 0)
	{
		LOG_TXT_DEBUG("all are sent.[%d][left:%d]", sock, buff->GetResponseLeft());

		/* ����BUFFΪ����״̬ */
		buff->SetStatus(CTServBuffer::E_IDLE); /* ���õľ�� */

		/* ҵ������� */
		EndProc(buff);

		/* �޸ľ�������¼�����ȡIN�¼����ȴ��������� */
		m_epoller->Mod(sock, EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP);
	}
	else
	{
		LOG_TXT_DEBUG("someone not sent.[%d]", sock);

		/* ���·�����Ӧ���ȴ��´η��� */
		m_rspbuf->Add((size_t)sock, buff);
	}

	return 0;
}

/*
 * Function: HandleErr
 * Desc: ����ͻ��˴���
 * In:
 *      int            sock  TCP���
 *      CTServBuffer * buff  ��Ӧ�������
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      -1    -   ʧ��
 */
int CTServFrm::HandleErr(int sock, CTServBuffer * buff /* = NULL */)
{
	if (sock == m_serv->GetFD())
	{
		/* ��epoll���Ƴ� */
		m_epoller->Remove(sock);

		/* �ر����ӻ��� */
		close(sock);

		LOG_TXT_INFO("close server[%d].", sock);

		return 0;
	}

	/* ȷ�Ͼ���Ƿ���ʹ���� */
	if (m_inuse->Get((size_t)sock) && (buff == NULL))
	{
		LOG_TXT_INFO("Biz Thread is alive now.[%d]", sock);
		return 0;
	}

	/* ȷ��BUFF״̬ */
	buff = ((buff == NULL) ? (CTServBuffer *)m_reqbuf->Remove((size_t)sock) : buff);
	buff = ((buff == NULL) ? (CTServBuffer *)m_rspbuf->Remove((size_t)sock) : buff);

	if (buff)
	{
		/* ����BUFFΪ����״̬ */
		buff->SetStatus(CTServBuffer::E_IDLE); /* ���õľ�� */

		/* ҵ������� */
		EndProc(buff);
	}

	/* ��ʱ��������� */
	m_timeout->UnUse((size_t)sock);

	/* ��epoll���Ƴ� */
	if (m_epoller->Remove(sock))
		return -1;

	/* �ر����ӻ��� */
	close(sock);

	/* ���ӿͻ���-1 */
	m_cltnum--;

	LOG_TXT_INFO("close a connection[%d].", sock);

	return 0;
}

/*
 * Function: ReloadConf
 * Desc: ���¼��������ļ�
 * In:
 *     const char *   conf  �����ļ���
 * Out:
 *     none
 * Return code:
 *     -1                   ����
 *      0                   �ɹ�
 */
int CTServFrm::ReloadConf(const char *conf)
{
	/* ����ϵͳһЩͳ����־ */
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
 * Desc: ����ͻ������
 * In:
 *      int            sock  TCP���
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      -1    -   ʧ��
 */
int CTServFrm::HandleOut(CTServBuffer * buff)
{
	LOG_TXT_DEBUG("unsent data.HandleOut: %d", buff->GetSock());

	/* ��buff������Ӧ���� */
	m_rspbuf->Add((size_t)buff->GetSock(), buff);

	/* �޸ľ�������¼�����ȡOUT�¼����ȴ��������� */
	m_epoller->Mod(buff->GetSock(), EPOLLOUT | EPOLLET | EPOLLERR | EPOLLHUP);

	return 0;
}

/*
 * Function: PreProc
 * Desc: Ԥ����
 * In:
 *      int            sock  TCP���
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      -1    -   ʧ��
 */
int CTServFrm::PreProc(CTServBuffer * buff)
{
	/* ��������봦����� */
	m_inuse->Add((size_t)(buff->GetSock()), buff);

	return 0;
}

/*
 * Function: EndProc
 * Desc: ��������
 * In:
 *      int            sock  TCP���
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      -1    -   ʧ��
 */
int CTServFrm::EndProc(CTServBuffer * buff)
{
	int sock = buff->GetSock();
	CTServBuffer::enuBuffStatus status = buff->GetStatus();

	LOG_TXT_DEBUG("end of the process.[sock:%d status:%d]", sock, status);

	/* ���buff״̬ */
	switch (status)
	{
	case CTServBuffer::E_IDLE:
		{
			/* �Ӵ�������Ƴ� */
			m_inuse->Remove((size_t)(buff->GetSock()));

			/* BUFF���ڿ���״̬��ҵ������ɣ����Ի��� */
			m_bufpool->release(buff);
		}
		break;
	case CTServBuffer::E_INUSE:
		{
			/* BUFF�д������ݣ���Ҫ���ͳ�ȥ,ҵ����δ��� */
			HandleOut(buff);
			return 0;
		}
		break;
	case CTServBuffer::E_UNUSED:
		{
			/* ҵ������ɣ�BUFF�������ѻ�����Ҫ�ر����� */
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
 * Desc: ����ʱ����
 * In:
 *      int            sock  TCP���
 * Out:
 *      none
 * Return code:
 *      0     -   �ɹ�
 *      -1    -   ʧ��
 */
int CTServFrm::HandleTimeout(int sock)
{
	/* �رվ������EPOLL����ʰ�ֳ� */
	shutdown(sock, SHUT_RDWR);

	LOG_TXT_INFO("timeout. close a connection[%d].", sock);

	return 0;
}
