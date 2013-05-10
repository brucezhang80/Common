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

/* Э��� */
#pragma pack(1)

typedef struct _tagTServProtoHead {
	unsigned int length; /* �����ȣ���ͷ+���� */
	unsigned int cmd; /* ������ */
	unsigned int sequence; /* ���к� */
} recTServProtoHead;

typedef struct _tagTServProtoPck {
	recTServProtoHead head; /* ����ͷ */
	char data[0]; /* ���� */
} recTServProtoPck;

#pragma pack()

class CTServBuffer;
class CTServFrm;
class CTSBizThreadPool;

/* ҵ�����߳� */
class CTSBizThread : public CLThread
{
	CTServFrm * m_tserv; /* TSERVER���� */
    CTSBizThreadPool * m_pool; /* ��������� */

	CTServBuffer * m_buff; /* ������� */

public:
    CTSBizThread(CTServFrm * tserv, CTSBizThreadPool * pool);
    ~CTSBizThread();

protected:
    /*
     * Function: HandleThread
     * Desc: �߳�ҵ����
     * In: 
     *      int    ibreak   �жϷ����־
     * Out: 
     *      int    ibreak   �жϷ����־
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    virtual int HandleThread(int& ibreak);

private:
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
    int Attach(CTServBuffer * buff);

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
    int Detach();

public:
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
    int Resume(CTServBuffer * buff);

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
    int Pause();

    /*
     * Function: GetBuffer
     * Desc: ��ȡ��������
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    CTServBuffer * GetBuffer() { return m_buff; };
};

class CTSBizThreadPool : public CObjPool<CTSBizThread>
{
    CTServFrm * m_tservfrm; /* ����TSERVER��� */
	CHash m_thread_list; /* �̹߳���� */
	int m_isstop; /* �Ƿ��˳� */

public:
    CTSBizThreadPool(CTServFrm * frm, int num);
    ~CTSBizThreadPool();

protected:
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
	CTSBizThread * find(pthread_t h);

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
    int Stop();
};

/* ����ʹ�û��� */
class CTServBuffer
{
	int m_buflen; /* ������󳤶� */

	char * m_request; /* ���󻺳� */
	int m_reqlen; /* ʵ���������ݳ��� */
	int m_reqleft; /* ʣ���������ݳ��� */

	char * m_response; /* ��Ӧ���� */
	int m_rspleft; /* ʣ����Ӧ���ݳ��� */

	CBaseSocket m_sock; /* ��Ӧ������ */

public:
	CTServBuffer(int buflen);
	~CTServBuffer();

public:
	typedef enum {
		E_IDLE = 0, /* ���п��� */
		E_INUSE, /* �������ݣ�ʹ���� */
		E_UNUSED, /* ���õģ������� */
	} enuBuffStatus;

private:
	enuBuffStatus m_status; /* ����״̬ */

public:
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
	int Attach(int sock);

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
	int Detach();

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
	int Recv(char * buf, int len);

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
	int Recv();

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
	int Send(const char * buf, int len);

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
	int Send();

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
	int Write(const char * buf, int len);

    /*
     * Function: GetRequest
     * Desc: �������󻺳�
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	const char * GetRequest() { return m_request; };

    /*
     * Function: GetRequestLength
     * Desc: �����������ݳ���
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int GetRequestLength() { return m_reqlen; };

    /*
     * Function: GetSock
     * Desc: ���ع���TCP���
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      >0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int GetSock() { return m_sock.GetFD(); };

	enuBuffStatus GetStatus() { return m_status; };
	int SetStatus(enuBuffStatus status) { m_status = status; return 0; };

	const char * GetResponse() { return m_response; };
	int GetResponseLeft() { return m_rspleft; };
};

class CTServBufferPool : public CObjPool<CTServBuffer>
{
    int m_buflen; /* ���ݳ��� */

public:
	CTServBufferPool(int num, int buflen);
	~CTServBufferPool();

protected:
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

/* ���Ӵ��� */
class CTServConn
{
	CTServFrm * m_tserv; /* �������� */
	int m_fd; /* epoll��� */

	int m_max_client; /* ���ͻ�����Ŀ */
	struct epoll_event * m_events; /* epoll�¼��б� */

public:
	CTServConn(CTServFrm * tserv, int max_client);
	~CTServConn();

public:
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
    int Handle();

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
    int Add(int sock, int flag = EPOLLIN |EPOLLOUT|EPOLLERR | EPOLLHUP);

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
    int Mod(int sock, int flag = EPOLLIN |EPOLLOUT|EPOLLERR | EPOLLHUP);

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
    int Remove(int sock);
};

class CTServTimeout: public CRealTime
{
	CTServFrm * m_tserv; /* ���� */

public:
	CTServTimeout(CTServFrm * tserv, int num, int timeout = -1);
	~CTServTimeout();

private:
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
	virtual int HandleTimeout(size_t key);
};

class CTServFrm : public CTFrm
{
	friend class CTSBizThread;

    CTServConf m_conf; /* �������� */

    CTSBizThreadPool * m_pool; /* ҵ�����̳߳� */

	CTServBufferPool * m_bufpool; /* ҵ������� */
	CHash * m_reqbuf; /* ���󻺳��,���ڴ��δ������Ļ������ */
	CHash * m_rspbuf; /* ��Ӧ�����,���ڴ��δ������Ļ������ */

    CSockAcceptor * m_serv; /* TCP SERVER ���� */
	CTServConn * m_epoller; /* ���Ӵ��� */

	CHash * m_inuse; /* �������ڴ���ľ�� */

	CTServTimeout * m_timeout; /* ��ʱ���� */

	/* ϵͳͳ����Ϣ */
	unsigned int m_cltnum; /* �ͻ�����Ŀ */
	unsigned long long m_reqnum; /* ���յ���������Ŀ */

public:
    CTServFrm(const char * name, const char * conf);
    ~CTServFrm();

protected:
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
    virtual int SetProcName(int argc, char * argv[]);

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
    virtual int InitConf(const char * conf);

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
    virtual int InitFrame();

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
    virtual int ReleaseFrame();

    /*
     * Function: HandleThread
     * Desc: �߳�ҵ����
     * In: 
     *      int    ibreak   �жϷ����־
     * Out: 
     *      int    ibreak   �жϷ����־
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    virtual int HandleThread(int& ibreak);

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
    virtual int OnStop();

    /*
     * Function: HandleClient
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
    int HandleClient(const char * buf, int buflen);

    /*
     * Function: nTServWrite
     * Desc: ������Ӧ����
     * In: 
     *      const char *   buf     ���ݻ���
	 *      int            buflen  ���ݳ���
     * Out: 
     *      none
     * Return code: 
     *      0     -   �ɹ�
     *      <0    -   ʧ��
     */
    static int nTServWrite(const char * buf, int buflen);

private:
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
    int AcceptConnect();

public:
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
    int HandleIn(int sock);

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
    int HandleOut(int sock);

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
    int HandleErr(int sock, CTServBuffer * buff = NULL);
 
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
    int ReloadConf(const char *conf);

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
    int HandleOut(CTServBuffer * buff);

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
    int PreProc(CTServBuffer * buff);

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
    int EndProc(CTServBuffer * buff);

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
    int HandleTimeout(int sock);

	/* ��ȡ�ؽӿ� */
    CTSBizThreadPool * GetTSBizThreadPool() const { return m_pool; }; /* ҵ�����̳߳� */
	CTServBufferPool * GetTServBufferPool() const { return m_bufpool; }; /* ҵ������� */
	CTServConn * GetTServConn() const { return m_epoller; }; /* ���Ӵ��� */
};

#endif
