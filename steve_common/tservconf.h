/*
 * @File: tservconf.h
 * @Desc: head file of TServer config
 * @Author: stevetang
 * @History:
 *      2007-11-27   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _TSERVCONF_H
#define _TSERVCONF_H

#include "conf.h"

#pragma pack(1)

/* 动态模块结构 */
#define MacMaxTModNameLen   64
#define MacMaxTModFileLen   512
#define MacMaxTModCmdLen    1024

/* 回调函数 */
typedef int (*pnTServCallBack)(const char * buf, int buflen);

/* 模块初始化 */
typedef int (*nTServModInit)(const char * conf);
/* 模块业务处理 */
typedef int (*nTServModDispatch)(const char * buf, int buflen, pnTServCallBack func);
/* 模块预退出，主要针对阻塞操作，唤醒阻塞线程 */
typedef int (*nTServModPreClose)();
/* 模块资源释放 */
typedef int (*nTServModDestroy)();

/* 发送数据 */

/* 组回送响应BUFFER */

typedef struct _tagTServModule {
	char name[MacMaxTModNameLen]; /* 模块名称 NAME */
	char modFile[MacMaxTModFileLen]; /* 模块文件 FILE */
	char modConf[MacMaxTModFileLen]; /* 模块配置文件 CONF */
	unsigned int modcmd_b; /* 开始命令码 CMDBEGIN */
	unsigned int modcmd_e; /* 结束命令码 CMDEND */
	char modInit[MacMaxTModNameLen]; /* 初始化接口名 MODINIT */
	char modDispatch[MacMaxTModNameLen]; /* 数据处理接口名 MODDISPATCH */
	char modPreClose[MacMaxTModNameLen]; /* 数据处理接口名 MODPRECLOSE */
	char modDestroy[MacMaxTModNameLen]; /* 释放资源接口名 MODDESTROY */

	void * modId; /* 模块句柄 */
	nTServModInit init; /* 初始化 */
	nTServModDispatch dispatch; /* 业务处理 */
	nTServModPreClose preclose; /* 模块预退出，主要针对阻塞操作，唤醒阻塞线程 */
	nTServModDestroy destroy; /* 模块释放 */
} recTServModule;

#pragma pack()

class CTServConf : public CConf
{
public:
    CTServConf(const char * lpConfFile);
    ~CTServConf();

public:
    /* 配置参数 */
    /* [SERVER] */
    char m_name[100]; /* SERVER名称 NAME */
	int m_bufnum; /* 缓冲数目 BUFNUM */
    int m_buflen; /* 缓冲大小 BUFLEN */
    int m_tnum; /* 线程数目 TNUM */
	char m_iname[100]; /* 网卡名称 INAME */
    char m_ip[16]; /* IP地址 SERVIP */
    int m_port; /* 端口 SERVPORT */
    int m_backlog; /* 侦听队列 BACKLOG */
    int m_timeout; /* 超时时间 TIMEOUT */
	int m_maxclt; /* 最大客户端连接数目 MAXCLTNUM */

    /* [LOG] */
	char m_log_path[512]; /* 日志路径 LOGPATH */
	char m_log_mod[512];   /* 日志模块名称 LOGMODNAME */
	int m_log_size; /* 日志文件大小 LOGFILESIZE */
	int m_log_cnt; /* 日志文件数目 LOGFILECNT */
    int m_log_level; /* 日志级别 LOGLEVEL */
	char m_log_filter[1024]; /* 日志过滤串 LOGFILTER */

	/* [MODULE] */
	int m_mod_num; /* 模块数目 MODNUM */

	/* [MODULE_?] */
	recTServModule * m_module; /* 模块列表 */

public:
    /*
     * Function: Init
     * Desc: 初始化系统参数
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Init();

    /*
     * Function: GetModule
     * Desc: 取得处理模块
     * In: 
     *      unsigned int   cmd   命令码
     * Out: 
     *      none
     * Return code: 
     *      非NULL  -   成功
     *      NULL    -   失败
     */
	recTServModule * GetModule(unsigned int cmd);

    /*
     * Function: Log
     * Desc: 系统参数写日志
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Log();
};

#endif
