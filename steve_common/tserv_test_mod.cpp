/*
 * @File: tserv_test_mod.cpp
 * @Desc: impliment file of TServer Test Module
 * @Author: stevetang
 * @History:
 *      2008-11-11   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "conf.h"
#include "log.h"

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

/* 回调函数 */
typedef int (*pnTServCallBack)(const char * buf, int buflen);

extern "C" {

char g_data[204800];

/* 模块初始化 */
int ModInit(const char * conf)
{
	/* 日志已做初始化，不需再初始化 */
	LOG_TXT_DEBUG("ModInit called.");

	memset(g_data, 0, sizeof(g_data));

	FILE * fp = fopen("1.txt", "rb");
	if (fp)
	{
		fread(g_data, 1, sizeof(g_data)-sizeof(recTServProtoPck)-1, fp);
		fclose(fp);
	}
	else
	{
		strcpy(g_data, "1234567890 mod ok.");
	}

	return 0;
}

/* 模块业务处理 */
int ModDispatch(const char * buf, int buflen, pnTServCallBack func)
{
	recTServProtoPck * rPck = (recTServProtoPck *)buf;

	LOG_TXT_DEBUG("ModDispatch called.length: %d cmd: %d sequence: %d", 
		rPck->head.length, rPck->head.cmd, rPck->head.sequence);

	char response[204800];

	memset(response, 0, sizeof(response));

	recTServProtoPck * rRsp = (recTServProtoPck *)response;

//	rRsp->head.length = sizeof(recTServProtoPck) + strlen("Tserv Test Module is running!");
	rRsp->head.length = sizeof(recTServProtoPck) + strlen(g_data);
	rRsp->head.cmd = rPck->head.cmd;
	rRsp->head.sequence = rPck->head.sequence;

//	strcpy(rRsp->data, "Tserv Test Module is running!");
	strcpy(rRsp->data, g_data);

#if 0
	FILE * fp = fopen("1.txt", "rb");
	if (fp)
	{
		fread(rRsp->data, 1, sizeof(response)-sizeof(recTServProtoPck)-1, fp);
		fclose(fp);

		rRsp->head.length = sizeof(recTServProtoPck) + strlen(rRsp->data);
	}
#endif

	func(response, rRsp->head.length);

	return 0;
}

/* 模块资源释放 */
int ModDestroy()
{
	LOG_TXT_DEBUG("ModDestroy called.");

	return 0;
}

}
