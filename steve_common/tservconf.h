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

/* ��̬ģ��ṹ */
#define MacMaxTModNameLen   64
#define MacMaxTModFileLen   512
#define MacMaxTModCmdLen    1024

/* �ص����� */
typedef int (*pnTServCallBack)(const char * buf, int buflen);

/* ģ���ʼ�� */
typedef int (*nTServModInit)(const char * conf);
/* ģ��ҵ���� */
typedef int (*nTServModDispatch)(const char * buf, int buflen, pnTServCallBack func);
/* ģ��Ԥ�˳�����Ҫ����������������������߳� */
typedef int (*nTServModPreClose)();
/* ģ����Դ�ͷ� */
typedef int (*nTServModDestroy)();

/* �������� */

/* �������ӦBUFFER */

typedef struct _tagTServModule {
	char name[MacMaxTModNameLen]; /* ģ������ NAME */
	char modFile[MacMaxTModFileLen]; /* ģ���ļ� FILE */
	char modConf[MacMaxTModFileLen]; /* ģ�������ļ� CONF */
	unsigned int modcmd_b; /* ��ʼ������ CMDBEGIN */
	unsigned int modcmd_e; /* ���������� CMDEND */
	char modInit[MacMaxTModNameLen]; /* ��ʼ���ӿ��� MODINIT */
	char modDispatch[MacMaxTModNameLen]; /* ���ݴ���ӿ��� MODDISPATCH */
	char modPreClose[MacMaxTModNameLen]; /* ���ݴ���ӿ��� MODPRECLOSE */
	char modDestroy[MacMaxTModNameLen]; /* �ͷ���Դ�ӿ��� MODDESTROY */

	void * modId; /* ģ���� */
	nTServModInit init; /* ��ʼ�� */
	nTServModDispatch dispatch; /* ҵ���� */
	nTServModPreClose preclose; /* ģ��Ԥ�˳�����Ҫ����������������������߳� */
	nTServModDestroy destroy; /* ģ���ͷ� */
} recTServModule;

#pragma pack()

class CTServConf : public CConf
{
public:
    CTServConf(const char * lpConfFile);
    ~CTServConf();

public:
    /* ���ò��� */
    /* [SERVER] */
    char m_name[100]; /* SERVER���� NAME */
	int m_bufnum; /* ������Ŀ BUFNUM */
    int m_buflen; /* �����С BUFLEN */
    int m_tnum; /* �߳���Ŀ TNUM */
	char m_iname[100]; /* �������� INAME */
    char m_ip[16]; /* IP��ַ SERVIP */
    int m_port; /* �˿� SERVPORT */
    int m_backlog; /* �������� BACKLOG */
    int m_timeout; /* ��ʱʱ�� TIMEOUT */
	int m_maxclt; /* ���ͻ���������Ŀ MAXCLTNUM */

    /* [LOG] */
	char m_log_path[512]; /* ��־·�� LOGPATH */
	char m_log_mod[512];   /* ��־ģ������ LOGMODNAME */
	int m_log_size; /* ��־�ļ���С LOGFILESIZE */
	int m_log_cnt; /* ��־�ļ���Ŀ LOGFILECNT */
    int m_log_level; /* ��־���� LOGLEVEL */
	char m_log_filter[1024]; /* ��־���˴� LOGFILTER */

	/* [MODULE] */
	int m_mod_num; /* ģ����Ŀ MODNUM */

	/* [MODULE_?] */
	recTServModule * m_module; /* ģ���б� */

public:
    /*
     * Function: Init
     * Desc: ��ʼ��ϵͳ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Init();

    /*
     * Function: GetModule
     * Desc: ȡ�ô���ģ��
     * In: 
     *      unsigned int   cmd   ������
     * Out: 
     *      none
     * Return code: 
     *      ��NULL  -   �ɹ�
     *      NULL    -   ʧ��
     */
	recTServModule * GetModule(unsigned int cmd);

    /*
     * Function: Log
     * Desc: ϵͳ����д��־
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Log();
};

#endif
