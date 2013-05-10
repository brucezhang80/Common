/*
 * route.h
 *
 *  Created on: 2010-6-11
 *      Author: evantang
 */

#ifndef _ROUTE_H_
#define _ROUTE_H_

#include "conf.h"
#include "tcp_client.h"
#include "KSSWord.h"
#include "oblist.h"
#include "log.h"
#include "KSSTservPck.h"

#define MAX_CONNECT_NUM 1000
#define SEND_RETRY_NUM 3

#define VALID 1
#define INVALID 0

typedef struct _tagE2DServInfo {
	char ip[16];
	int port;
	int connect_num; /*������*/
} recE2DServInfo;

class CRouteConf : public CConf
{
public:
	CRouteConf(const char * lpConfFile);
	~CRouteConf();

private:
	/*
	 * Function: Init
	 * Desc: ��ʼ��������Ϣ
	 * In:
	 *     const char *   lpConfFile  �ļ���
	 * Out:
	 *     none
	 * Return code:
	 *     -1                   ����
	 *     0                    �ɹ�
	 */
	int Init(const char * lpConfFile);

public:
	int m_servnum; /* ��������Ŀ */
	int m_timeout; /* ��ʱʱ�� */

	recE2DServInfo * m_servInfo; /* ��������Ϣ */
};

class CRoute
{
public:
	CRoute(int _timeout = -1);
	~CRoute();

private:
	//��ȡ����
	void * GetConnection();

	//��������
	int Send(const char * buf, int len);

	//���ͺͽ�������
	int SendAndRecv(char * buf, int len, char * recv, int& recv_len);

public:
	//��ʼ��
	int Init(const char *pszConfFile);

	//����ʼ��
	int Exit();

	/* ��ȡ��Ӧword��Ӧ�Ĳ�ѯ���� */
	int Deal(char * recv, int& recv_len, int cmd, const char* word, const enuKSSWordType *type, const int *start_pos, const int *end_pos, const int *time_stamp = NULL);

	/* ֱ�Ӱ�recv������ݶ�Ӧ���ߣ�recv��Ҫ��հ�ͷ */
	int Deal(char * recv, int& recv_len, const char* buf, const int buf_len);

	/* ��ȡ��word��Ӧ��docid���� */
	int GetDocId(char * recv, int& recv_len, const char* word, const unsigned long long *docid);

	/* ���¸ôʼ�����Ӧ������*/
	int SaveDocId(char * recv, int& recv_len, const char* word, const unsigned long long *docid, const char* save_buf, const int save_buf_len);

	/* ����·�ɲ����� */
	void SetInvalid()	{valid = INVALID;}

	/* ����·�ɿ��� */
	void SetValid()		{valid = VALID;}

	/* �ж��Ƿ����*/
	bool IsValid()
	{
		if (valid == VALID)
			return true;
		else
			return false;
	}

	/* ��ȡ���������ip��Ϣ*/
	const char* GetIp()
	{
		/*���û�����ӣ����ؿ�ֵ*/
		if (g_conf->m_servnum <= 0)
			return NULL;
		/*���򷵻ص�1��ip��Ϣ*/
		else
			return g_conf->m_servInfo->ip;
	}

	/*�ж��Ƿ�������������*/
	bool HasMachine()
	{
		if (g_conf->m_servnum <= 0)
			return false;
		else
			return true;
	}

private:
	// modified by evantang 2010-04-06, change global var to local var.
	CRouteConf * g_conf;
	CObList<CTcpClient> *g_list; /* TCP���� */

	int valid;
	int time_out; /* ���ճ�ʱʱ�� */

private:
	int GetSequence(const int port)
	{
		timeval start_t;
		gettimeofday(&start_t, NULL);

		char temp[100] = {0};
		LOG_TXT_DEBUG("port: %d, start_t: %d", port, start_t.tv_usec);
		sprintf(temp, "%d%d", port, start_t.tv_usec % 10000);
		LOG_TXT_DEBUG("merge: %s", temp);
		int seq = atoi(temp);
		return seq;
	}
};

#endif /* ROUTE_H_ */
