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
	int connect_num; /*连接数*/
} recE2DServInfo;

class CRouteConf : public CConf
{
public:
	CRouteConf(const char * lpConfFile);
	~CRouteConf();

private:
	/*
	 * Function: Init
	 * Desc: 初始化配置信息
	 * In:
	 *     const char *   lpConfFile  文件名
	 * Out:
	 *     none
	 * Return code:
	 *     -1                   出错
	 *     0                    成功
	 */
	int Init(const char * lpConfFile);

public:
	int m_servnum; /* 服务器数目 */
	int m_timeout; /* 超时时间 */

	recE2DServInfo * m_servInfo; /* 服务器信息 */
};

class CRoute
{
public:
	CRoute(int _timeout = -1);
	~CRoute();

private:
	//获取链接
	void * GetConnection();

	//发送数据
	int Send(const char * buf, int len);

	//发送和接收数据
	int SendAndRecv(char * buf, int len, char * recv, int& recv_len);

public:
	//初始化
	int Init(const char *pszConfFile);

	//反初始化
	int Exit();

	/* 获取对应word相应的查询内容 */
	int Deal(char * recv, int& recv_len, int cmd, const char* word, const enuKSSWordType *type, const int *start_pos, const int *end_pos, const int *time_stamp = NULL);

	/* 直接把recv里的内容对应发走，recv需要封闭包头 */
	int Deal(char * recv, int& recv_len, const char* buf, const int buf_len);

	/* 获取与word对应的docid内容 */
	int GetDocId(char * recv, int& recv_len, const char* word, const unsigned long long *docid);

	/* 存下该词极其相应的内容*/
	int SaveDocId(char * recv, int& recv_len, const char* word, const unsigned long long *docid, const char* save_buf, const int save_buf_len);

	/* 设置路由不可用 */
	void SetInvalid()	{valid = INVALID;}

	/* 设置路由可用 */
	void SetValid()		{valid = VALID;}

	/* 判断是否可用*/
	bool IsValid()
	{
		if (valid == VALID)
			return true;
		else
			return false;
	}

	/* 获取该连接类的ip信息*/
	const char* GetIp()
	{
		/*如果没有连接，返回空值*/
		if (g_conf->m_servnum <= 0)
			return NULL;
		/*否则返回第1个ip信息*/
		else
			return g_conf->m_servInfo->ip;
	}

	/*判断是否有连接数存在*/
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
	CObList<CTcpClient> *g_list; /* TCP连接 */

	int valid;
	int time_out; /* 接收超时时间 */

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
