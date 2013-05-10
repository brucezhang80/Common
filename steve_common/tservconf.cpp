/*
 * @File: tservconf.cpp
 * @Desc: implement file of TServer config
 * @Author: stevetang
 * @History:
 *      2007-11-27   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "common.h"
#include "tservconf.h"
#include "log.h"

CTServConf::CTServConf(const char * lpConfFile) : CConf(lpConfFile), m_module(NULL)
{
    /* 初始化系统参数 */
//    Init();
}

CTServConf::~CTServConf()
{
	/* 释放模块信息 */
	if (m_module)
	{
		free(m_module);
		m_module = NULL;
	}
}

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
int CTServConf::Init()
{
    /* SERVER */
    memset(m_name, 0, sizeof(m_name));
    GetStr("SERVER", "NAME", m_name, sizeof(m_name), "TServer");
    GetInt("SERVER", "BUFNUM", &m_bufnum, 20);
    GetInt("SERVER", "BUFLEN", &m_buflen, 1024);
    GetInt("SERVER", "TNUM", &m_tnum, 10);
    memset(m_iname, 0, sizeof(m_iname));
    GetStr("SERVER", "INAME", m_iname, sizeof(m_iname), "eth1");
    GetInt("SERVER", "SERVPORT", &m_port, 9008);
    GetInt("SERVER", "BACKLOG", &m_backlog, 10);
    GetInt("SERVER", "TIMEOUT", &m_timeout, 120000);
    GetInt("SERVER", "MAXCLTNUM", &m_maxclt, 1024);

    memset(m_ip, 0, sizeof(m_ip));
    GetIpByIf(m_iname, m_ip);

	/* LOG信息 */
    GetStr("LOG", "LOGPATH", m_log_path, sizeof(m_log_path), "../log");
    GetStr("LOG", "LOGMODNAME", m_log_mod, sizeof(m_log_mod), "TServer");
    GetInt("LOG", "LOGFILESIZE", &m_log_size, 10);
    GetInt("LOG", "LOGFILECNT", &m_log_cnt, 10);
    GetInt("LOG", "LOGLEVEL", &m_log_level, 8);
    GetStr("LOG", "LOGFILTER", m_log_filter, sizeof(m_log_filter), "");

	/* MODULE信息 */
    GetInt("MODULE", "MODNUM", &m_mod_num, 0);

	m_module = (recTServModule *)malloc(sizeof(recTServModule) * m_mod_num);
	if (m_module == NULL)
		return -1;
	memset((char *)m_module, 0, sizeof(recTServModule) * m_mod_num);

	/* MODULE详细信息 */
	for (int i=0; i<m_mod_num; i++)
	{
		char name[100];
		recTServModule * module = (recTServModule *)&m_module[i];

		sprintf(name, "MODULE_%d", i+1);

		GetStr(name, "NAME", module->name, sizeof(module->name), "");
		GetStr(name, "FILE", module->modFile, sizeof(module->modFile), "");
		GetStr(name, "CONF", module->modConf, sizeof(module->modConf), "");
		GetInt(name, "CMDBEGIN", (int *)&module->modcmd_b, 0);
		GetInt(name, "CMDEND", (int *)&module->modcmd_e, 0);

		GetStr(name, "MODINIT", module->modInit, sizeof(module->modInit), "");
		GetStr(name, "MODDISPATCH", module->modDispatch, sizeof(module->modDispatch), "");
		GetStr(name, "MODPRECLOSE", module->modPreClose, sizeof(module->modPreClose), "");
		GetStr(name, "MODDESTROY", module->modDestroy, sizeof(module->modDestroy), "");
	}

    /* 打印参数 */
    printf(".............................................\n");

    printf("NAME: %s\n", m_name);
    printf("BUFNUM: %d\n", m_bufnum);
    printf("BUFLEN: %d\n", m_buflen);
    printf("TNUM: %d\n", m_tnum);
    printf("SERVIP: %s\n", m_ip);
    printf("SERVPORT: %d\n", m_port);
    printf("BACKLOG: %d\n", m_backlog);
    printf("TIMEOUT: %d\n", m_timeout);
	printf("MAXCLTNUM: %d\n", m_maxclt);

    printf("LOGPATH: %s\n", m_log_path);
    printf("LOGMODNAME: %s\n", m_log_mod);
    printf("LOGFILESIZE: %d\n", m_log_size);
    printf("LOGFILECNT: %d\n", m_log_cnt);
    printf("LOGLEVEL: %d\n", m_log_level);
    printf("LOGFILTER: %s\n", m_log_filter);

    printf("MODNUM: %d\n", m_mod_num);

	for (int i=0; i<m_mod_num; i++)
	{
		recTServModule * module = (recTServModule *)&m_module[i];

		printf("----------------------------------------------\n");
		printf("NAME: %s\n", module->name);
		printf("FILE: %s\n", module->modFile);
		printf("CONF: %s\n", module->modConf);
		printf("CMDBEGIN: %u\n", module->modcmd_b);
		printf("CMDEND: %u\n", module->modcmd_e);
		printf("MODINIT: %s\n", module->modInit);
		printf("MODDISPATCH: %s\n", module->modDispatch);
		printf("MODPRECLOSE: %s\n", module->modPreClose);
		printf("MODDESTROY: %s\n", module->modDestroy);
		printf("----------------------------------------------\n");
	}
    printf(".............................................\n");

    return 0;
}

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
recTServModule * CTServConf::GetModule(unsigned int cmd)
{
	recTServModule * mod = NULL;

	for (int i=0; i<m_mod_num; i++)
	{
		recTServModule * module = (recTServModule *)&m_module[i];

		if (cmd >= module->modcmd_b && cmd <= module->modcmd_e)
		{
			mod = module;
			break;
		}
	}

	return mod;
}

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
int CTServConf::Log()
{
    /* 打印参数 */
    LOG_TXT_INFO(".............................................");

    LOG_TXT_INFO("NAME: %s", m_name);
    LOG_TXT_INFO("BUFNUM: %d", m_bufnum);
    LOG_TXT_INFO("BUFLEN: %d", m_buflen);
    LOG_TXT_INFO("TNUM: %d", m_tnum);
    LOG_TXT_INFO("SERVIP: %s", m_ip);
    LOG_TXT_INFO("SERVPORT: %d", m_port);
    LOG_TXT_INFO("BACKLOG: %d", m_backlog);
    LOG_TXT_INFO("TIMEOUT: %d", m_timeout);
	LOG_TXT_INFO("MAXCLTNUM: %d", m_maxclt);

    LOG_TXT_INFO("LOGPATH: %s", m_log_path);
    LOG_TXT_INFO("LOGMODNAME: %s", m_log_mod);
    LOG_TXT_INFO("LOGFILESIZE: %d", m_log_size);
    LOG_TXT_INFO("LOGFILECNT: %d", m_log_cnt);
    LOG_TXT_INFO("LOGLEVEL: %d", m_log_level);
    LOG_TXT_INFO("LOGFILTER: %s", m_log_filter);

    LOG_TXT_INFO("MODNUM: %d", m_mod_num);

	for (int i=0; i<m_mod_num; i++)
	{
		recTServModule * module = (recTServModule *)&m_module[i];

		LOG_TXT_INFO("----------------------------------------------");
		LOG_TXT_INFO("NAME: %s", module->name);
		LOG_TXT_INFO("FILE: %s", module->modFile);
		LOG_TXT_INFO("CONF: %s", module->modConf);
		LOG_TXT_INFO("CMDBEGIN: %u", module->modcmd_b);
		LOG_TXT_INFO("CMDEND: %u", module->modcmd_e);
		LOG_TXT_INFO("MODINIT: %s", module->modInit);
		LOG_TXT_INFO("MODDISPATCH: %s", module->modDispatch);
		LOG_TXT_INFO("MODPRECLOSE: %s", module->modPreClose);
		LOG_TXT_INFO("MODDESTROY: %s", module->modDestroy);
		LOG_TXT_INFO("----------------------------------------------");
	}
    LOG_TXT_INFO(".............................................");

	return 0;
}
