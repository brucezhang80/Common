/*
 * route.cpp
 *
 *  Created on: 2010-6-11
 *      Author: evantang
 */
#include "route.h"

int wait_recnt, succ_recnt = 0;

CRouteConf::CRouteConf(const char * lpConfFile) : CConf(lpConfFile)
{
	Init(lpConfFile);
}

CRouteConf::~CRouteConf()
{
	if (m_servInfo)
	{
		free(m_servInfo);
		m_servInfo = NULL;
	}
}

int CRouteConf::Init(const char * lpConfFile)
{
	/* 取服务器数目 */
	GetInt("SYSTEM", "SERVNUM", &m_servnum, 0);

	/* 取超时时间 */
	GetInt("SYSTEM", "TIMEOUT", &m_timeout, 120000);

	/* 取服务器信息 */
	m_servInfo = (recE2DServInfo *)malloc(sizeof(recE2DServInfo) * m_servnum);
	if (m_servInfo == NULL)
	{
		LOG_TXT_ERR("Malloc Memory for Route Initi Error.");
		return -1;
	}

	for (int i=0; i<m_servnum; i++)
	{
		char section[1024];
		sprintf(section, "SERV_%d", i+1);

		/* 取服务器IP地址 */
		GetStr(section, "SERVIP", m_servInfo[i].ip, sizeof(m_servInfo[i].ip), "172.0.0.1");

		/* 取服务器端口 */
		GetInt(section, "SERVPORT", &m_servInfo[i].port, 9010);

		/* 取连接数 */
		GetInt(section, "SERVCONNECTNUM", &m_servInfo[i].connect_num, 1);
	}


	LOG_TXT_INFO("*******************************************************\n");
	LOG_TXT_INFO("SERVNUM: %d\n", m_servnum);
	LOG_TXT_INFO("TIMEOUT: %d\n", m_timeout);

	for (int i=0; i<m_servnum; i++)
	{
		LOG_TXT_INFO("SERVIP[%d]: %s\n", i+1, m_servInfo[i].ip);
		LOG_TXT_INFO("SERVPORT[%d]: %d\n", i+1, m_servInfo[i].port);
	}
	LOG_TXT_INFO("*******************************************************\n");

	return 0;
}

CRoute::CRoute(int _timeout/* = -1*/)
{
	g_conf = NULL;
	g_list = NULL;

	time_out = _timeout;
	valid = VALID;
}

CRoute::~CRoute()
{
	Exit();
}

//初始化
int CRoute::Init(const char *pszConfFile)
{
	/* 读取配置文件 */
	g_conf = new CRouteConf(pszConfFile);
	if (g_conf == NULL)
	{
		LOG_TXT_ERR("Initi CRouteConf Error.");
		return -1;
	}

	/* 创建TCP链接池 */
	g_list = new CObList<CTcpClient>(g_conf->m_servnum);
	if (g_list == NULL)
	{
		LOG_TXT_ERR("Initi CTcpClient Pool Error.");
		return -2;
	}

	for (int i = 0; i < g_conf->m_servnum; i++)
	{
		if (g_conf->m_servInfo[i].connect_num > MAX_CONNECT_NUM)
			g_conf->m_servInfo[i].connect_num = MAX_CONNECT_NUM;
		for (int j = 0; j < g_conf->m_servInfo[i].connect_num; j++)
		{
			/* 创建TCP链接 */
			CTcpClient  * clt = new CTcpClient(g_conf->m_servInfo[i].ip, g_conf->m_servInfo[i].port,2000);

			if (clt == NULL)
			{
				LOG_TXT_ERR("Initi Tcp Client Error in IP[%s] Port[%d]", g_conf->m_servInfo[i].ip, g_conf->m_servInfo[i].port);
				return -3;
			}

			/* 放入连接池 */
			g_list->release(clt);
		}
	}
	LOG_TXT_INFO("Initi CRoute Connect [%d] Finished.", g_conf->m_servnum);
	return 0;
}

//反初始化
int CRoute::Exit()
{
	/* 删除连接池 */
	if (g_list)
	{
		delete g_list;
		g_list = NULL;
	}

	/* 删除配置对象 */
	if (g_conf)
	{
		delete g_conf;
		g_conf = NULL;
	}
	LOG_TXT_INFO("CRoute Exited.");
	return 0;
}

//获取链接
void * CRoute::GetConnection()
{
	return g_list->get();
}

//发送数据
	// modified by evantang 2010-04-02,增加type字段，用于标识web数据及wob数据
int CRoute::Send(const char * buf, int len)
{
SEND_RETRY:
	CTcpClient * pClt = (CTcpClient*)GetConnection();
	if (pClt == NULL)
	{
		LOG_TXT_INFO("Get Connection Error");
		return -1;
	}

	char ip[100];
	memset(ip, 0, sizeof(ip));
	pClt->GetPeerIP(ip, sizeof(ip));

	unsigned int nWriteBytes = len;

	int nRet = pClt->SendN(nWriteBytes, buf, (unsigned int)len);

	LOG_TXT_INFO("Send[%s]: %08X %d\n", ip, pClt, nRet);

	if (nRet <= 0)
	{
		LOG_TXT_ERR("Connection error.[%s]\n", ip);

		/* 重连 */
		pClt->Reconnect();

		/* 放入连接池 */
		g_list->release(pClt);

		goto SEND_RETRY;
	}


	/* 放入连接池 */
	g_list->release(pClt);

	return 0;
}

//发送和接收数据
int CRoute::SendAndRecv(char * buf, int len, char * recv, int& recv_len)
{
	int retry_num = 0;
	CTcpClient * pClt = NULL;
	char ip[100];

	/*阻塞模式 一定可以申请到*/
	pClt = (CTcpClient*)GetConnection();
	if (pClt == NULL)
	{
		LOG_TXT_ERR("Get Connection Error");
		return -1;
	}

	/*增加序列号字段*/
	recRequestPck * rReqPck = (recRequestPck *)buf;
	int seq = GetSequence(pClt->GetSockPort());
	rReqPck->head.magic = seq;
	LOG_TXT_DEBUG("seq num : %d", seq);

	memset(ip, 0, sizeof(ip));
	pClt->GetPeerIP(ip, sizeof(ip));

SENDRETRY:

	if (retry_num == SEND_RETRY_NUM)
	{
		LOG_TXT_ERR("send error for ip[%s], try [%d] times.", ip, SEND_RETRY_NUM);

		/* 放入连接池 */
		g_list->release(pClt);

		return SendError;
	}

	retry_num++;

	unsigned int nWriteBytes = len;

	LOG_TXT_DEBUG("*****************send to***************************");
	LOG_BIN_DEBUG(buf, (unsigned int)len);
	LOG_TXT_DEBUG("*****************send to***************************");

	int nRet = pClt->SendN(nWriteBytes, buf, (unsigned int)len);

	LOG_TXT_DEBUG("Send[%s]: %08X %d\n", ip, pClt, nRet);

	if (nRet <= 0)
	{
		LOG_TXT_ERR("Connection error.[%s][%d]\n", ip, nRet);

		/* 重连 */
		pClt->Reconnect();

		goto SENDRETRY;
	}

	/*接收数据*/
	unsigned int nReadBytes = sizeof(recResponsePck);

	/*收包头，连head带后面的ret一起收*/
	recResponsePck *resp = (recResponsePck *)recv;
	int ret = pClt->RecvN(nReadBytes, &resp->head, nReadBytes, time_out);
	if (ret != sizeof(recResponsePck))
	{
		LOG_TXT_ERR("Fail to recv head of response, Return.[%s][%d][%d][%s]\n", ip, ret, pClt->GetFD(), strerror(errno));

		/* 重连 */
		pClt->Reconnect();

		goto SENDRETRY;
		//return ReceiveSearchHeadError;
	}

	/*判断长度是否足够*/
	nReadBytes = resp->head.length - sizeof(recResponsePck);

	LOG_TXT_DEBUG("Pck Length[%d], Data length[%d]", resp->head.length, nReadBytes);

	/*只回了一个包头  表示有错误的情况，不用再收包体了*/
	if (nReadBytes == 0)
	{
		if (resp->ret != 0)
		{
			LOG_TXT_ERR("Search Error, Code is [%d]", resp->ret);
		}
		else
		{
			LOG_TXT_INFO("Receive Correct Signal");
		}
	}
	/*长度超过*/
	else if (nReadBytes >= recv_len - sizeof(recResponsePck))
	{
		LOG_TXT_ERR("Recv buff[%d] is too long for Remain buff[%d]", recv_len, nReadBytes);
		g_list->release(pClt);
		return NotEnoughBuff;
	}
	else
	{
		/*收包体*/
		ret = pClt->RecvN(nReadBytes, resp->data, nReadBytes, time_out);
		if (ret != (int)(resp->head.length - sizeof(recResponsePck)))
		{
			LOG_TXT_ERR("Fail to recv body of response, Return.[%s][%d][%d][%s]\n", ip, ret, pClt->GetFD(), strerror(errno));

			/* 重连 */
			pClt->Reconnect();

			goto SENDRETRY;
			//return ReceiveSearchHeadError;
		}

		if (resp->head.magic != seq)
		{
			LOG_TXT_ERR("recv seq [%d] is not equal with send seq [%d]", resp->head.magic, seq);

			/* 重连 */
			pClt->Reconnect();

			goto SENDRETRY;
			//return ReceiveSearchHeadError;
		}
	}
	/*置回处理长度*/
	recv_len = resp->head.length;
	LOG_TXT_DEBUG("Recv[%s]: %08X %d\n", ip, pClt, recv_len);

	/* 放入连接池 */
	g_list->release(pClt);

	return 0;
}

/* 获取对应word相应的查询内容 */
int CRoute::Deal(char * recv, int& recv_len, int cmd, const char* word,const enuKSSWordType *type, const int *start_pos, const int *end_pos, const int *time_stamp/* = NULL*/)
{
	char buf[2048] = {0};
	recRequestPck * rReqPck = (recRequestPck *)buf;

	rReqPck->head.cmd = cmd;
	rReqPck->head.magic = 0;
	rReqPck->head.length = 0;

	int length = sizeof(recRequestPck);

	char *p = rReqPck->data;

	int word_len = strlen(word);
	if (word_len > MAX_WORD_LENGTH)
	{
		LOG_TXT_ERR("Deal:Word Length[%d] too long.", word_len);
		return WordLengthTooLong;
	}

	/*关键词类型*/
	memcpy(p, type, sizeof(enuKSSWordType));
	p += sizeof(enuKSSWordType);
	length += sizeof(enuKSSWordType);

	/*关键词长度*/
	memcpy(p, &word_len, sizeof(int));
	p += sizeof(int);
	length += sizeof(int);

	/*关键词*/
	memcpy(p, word, word_len);
	p += word_len;
	length += word_len;

	/*起始条目号*/
	memcpy(p, start_pos, sizeof(int));
	p += sizeof(int);
	length += sizeof(int);


	/*终止条目号*/
	memcpy(p, end_pos, sizeof(int));
	p += sizeof(int);
	length += sizeof(int);

	/*时间戳 可选*/
	if (time_stamp != NULL)
	{
		memcpy(p, time_stamp, sizeof(int));
		p += sizeof(int);
		length += sizeof(int);
	}

	rReqPck->head.length = length;

	return SendAndRecv(buf, length, recv, recv_len);
}

/* 获取与word对应的docid内容 */
int CRoute::GetDocId(char * recv, int& recv_len, const char* word, const unsigned long long *docid)
{
	char buf[2048] = {0};
	recRequestPck * rReqPck = (recRequestPck *)buf;

	rReqPck->head.cmd = SEARCHKSSDOC;
	rReqPck->head.magic = 0;
	rReqPck->head.length = 0;

	int length = sizeof(recRequestPck);

	char *p = rReqPck->data;

	int word_len = strlen(word);
	if (word_len > MAX_WORD_LENGTH)
	{
		LOG_TXT_ERR("Deal:Word Length[%d] too long.", word_len);
		return WordLengthTooLong;
	}

	enuKSSWordType type = VIDEO; /*随便填一个type 方便解包*/
	/*关键词类型*/
	memcpy(p, &type, sizeof(enuKSSWordType));
	p += sizeof(enuKSSWordType);
	length += sizeof(enuKSSWordType);

	/*关键词长度*/
	memcpy(p, &word_len, sizeof(int));
	p += sizeof(int);
	length += sizeof(int);

	/*关键词*/
	memcpy(p, word, word_len);
	p += word_len;
	length += word_len;

	/*DocId*/
	memcpy(p, docid, sizeof(unsigned long long));
	p += sizeof(unsigned long long);
	length += sizeof(unsigned long long);

	rReqPck->head.length = length;

	LOG_TXT_DEBUG("GetDocId: Send[%d]", length);

	return SendAndRecv(buf, length, recv, recv_len);
}

int CRoute::Deal(char * recv, int& recv_len, const char* buf, const int buf_len)
{
	char *c_buf = (char*)buf;
	return SendAndRecv(c_buf, buf_len, recv, recv_len);
}

/* 存下该词极其相应的内容*/
int CRoute::SaveDocId(char * recv, int& recv_len, const char* word, const unsigned long long *docid, const char* save_buf, const int save_buf_len)
{
	char buf[2048] = {0};
	unsigned int buf_len = sizeof(buf);

	if (buf_len < save_buf_len + sizeof(unsigned long long) + strlen(word) + sizeof(int))
	{
		LOG_TXT_ERR("DocId[%llu] Content too long", *docid);
		return NotEnoughBuff;
	}

	recRequestPck * rReqPck = (recRequestPck *)buf;

	rReqPck->head.cmd = SAVESINGLEDOCID;
	rReqPck->head.magic = 0;
	rReqPck->head.length = 0;

	int length = sizeof(recRequestPck);

	char *p = rReqPck->data;

	int word_len = strlen(word);
	if (word_len > MAX_WORD_LENGTH)
	{
		LOG_TXT_ERR("Deal:word[%s] length[%d] too long.", word, word_len);
		return WordLengthTooLong;
	}

	/*关键词长度*/
	memcpy(p, &word_len, sizeof(int));
	p += sizeof(int);
	length += sizeof(int);

	/*关键词*/
	memcpy(p, word, word_len);
	p += word_len;
	length += word_len;

	/*DocId*/
	memcpy(p, docid, sizeof(unsigned long long));
	p += sizeof(unsigned long long);
	length += sizeof(unsigned long long);

	/*内容*/
	memcpy(p, save_buf, save_buf_len);
	p += save_buf_len;
	length += save_buf_len;

	rReqPck->head.length = length;

	LOG_TXT_DEBUG("SaveDocId: Send[%d]", length);

	return SendAndRecv(buf, length, recv, recv_len);
}

