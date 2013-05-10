/*
 * @File: log.h
 * @Desc: head file of log
 * @Author: stevetang
 * @History:
 *      2008-10-16   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _LOG_H
#define _LOG_H

/***********************************************��־������*********************************************************/
//ֵ	|����		|����								|�ı���洢ʱ�������Զ�̬���ã�		|�������־���鼶��
//0		|PANIC		|��ϵͳ������						|3��					|
//1		|ALERT		|��Ҫ�������޸ĵ�����				|2��					|
//2		|CRIT		|��ֹĳЩ���߻���ϵͳ����ʵ�ֵĴ���
//                  |����				                |1��					|
//3		|ERR		|��ֹ���߻�ĳЩ��ϵͳ���ֹ���ʵ�ֵ�
//                  |��������			                |6����					|�쳣��
//4		|WARNING	|Ԥ����Ϣ							|3����					|�߼�������
//5		|NOTICE		|������Ҫ�Ե���ͨ����				|2����					|��ͨ״̬��
//6		|INFO		|�ṩ��Ϣ����Ϣ						|1����					|Ĭ��
//7		|DEBUG		|���������������������������Ϣ		|15��					|
//8		|NONE		|û����Ҫ����ͨ�������Ŵ�			|1��					|

#ifndef MAINTENANCE_LOG_LEVEL_PANIC
#define MAINTENANCE_LOG_LEVEL_PANIC	0	//��ϵͳ������
#endif

#ifndef MAINTENANCE_LOG_LEVEL_ALERT
#define MAINTENANCE_LOG_LEVEL_ALERT	1	//��Ҫ�������޸ĵ�����
#endif

#ifndef MAINTENANCE_LOG_LEVEL_CRIT
#define MAINTENANCE_LOG_LEVEL_CRIT	2	//��ֹĳЩ���߻���ϵͳ����ʵ�ֵĴ�������
#endif

#ifndef MAINTENANCE_LOG_LEVEL_ERR
#define MAINTENANCE_LOG_LEVEL_ERR	3	//��ֹ���߻�ĳЩ��ϵͳ���ֹ���ʵ�ֵĴ�������
#endif

#ifndef MAINTENANCE_LOG_LEVEL_WARNING
#define MAINTENANCE_LOG_LEVEL_WARNING	4	//Ԥ����Ϣ
#endif

#ifndef MAINTENANCE_LOG_LEVEL_NOTICE
#define MAINTENANCE_LOG_LEVEL_NOTICE	5	//������Ҫ�Ե���ͨ����
#endif

#ifndef MAINTENANCE_LOG_LEVEL_INFO
#define MAINTENANCE_LOG_LEVEL_INFO	6	//�ṩ��Ϣ����Ϣ
#endif

#ifndef MAINTENANCE_LOG_LEVEL_DEBUG
#define MAINTENANCE_LOG_LEVEL_DEBUG	7	//���������������������������Ϣ
#endif

#ifndef MAINTENANCE_LOG_LEVEL_NONE
#define MAINTENANCE_LOG_LEVEL_NONE	8	//û����Ҫ����ͨ�������Ŵ�
#endif

#define MacMaxFilterLen     1024

class CLog
{
	int m_level; /* ��־���� */
	char m_filter[MacMaxFilterLen]; /* Filter�� */
	static void * m_pCycLog; /* ��־���� */

public:
	CLog();
	~CLog();

public:
    /*
     * Function: Init
     * Desc: ��ʼ��
     * In: 
     *      const char *   logpath      �ļ���
     *      const char *   modname      ������
     *      int			   max_filesize ����ļ�����
     *      int			   max_filecnt  ����ļ���Ŀ
	 *      int            level        ��־����
     *      const char *   filter       ���˴�
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int Init(const char * logpath, const char * modname, int max_filesize, int max_filecnt, int level = MAINTENANCE_LOG_LEVEL_NONE, const char * filter = "");

	/*
     * Function: Write
     * Desc: д�ı���־
     * In: 
	 *      int            level     ��־����
     *      const char *   file      �ļ���
     *      const char *   func      ������
     *      const int      line      �к�
     *      const char *   msg       ��־��Ϣ
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    int Write(int level, const char * file, const char * func, const int line, const char * msg);

    /*
     * Function: Write
     * Desc: д��������־
     * In: 
	 *      int            level     ��־����
     *      const char *   file      �ļ���
     *      const char *   func      ������
     *      const int      line      �к�
     *      const char *   msg       ��־��Ϣ
     *      const int      msglen    ��Ϣ����
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    int Write(int level, const char * file, const char * func, const int line, const char * msg, const int msglen);
};

extern CLog m_log;

#define MAX_LOG_MSGLEN      8192

#define LOG_MSG(fmt, args...) \
            char msglog[MAX_LOG_MSGLEN]; \
            snprintf(msglog, MAX_LOG_MSGLEN-1, fmt, ##args);

/* ��ʼ����־ */
#define LOG_INIT(logpath, modname, max_filesize, max_filecnt, args...) \
    { \
        m_log.Init(logpath, modname, max_filesize, max_filecnt, ##args); \
    }

/* д�ı���־ */
#define LOG_TXT_PANIC(fmt, args...) \
    { \
        LOG_MSG(fmt, ##args) \
        m_log.Write(MAINTENANCE_LOG_LEVEL_PANIC, __FILE__, __FUNCTION__, __LINE__, msglog); \
    }

#define LOG_TXT_ALERT(fmt, args...) \
    { \
        LOG_MSG(fmt, ##args) \
        m_log.Write(MAINTENANCE_LOG_LEVEL_ALERT, __FILE__, __FUNCTION__, __LINE__, msglog); \
    }

#define LOG_TXT_CRIT(fmt, args...) \
    { \
        LOG_MSG(fmt, ##args) \
        m_log.Write(MAINTENANCE_LOG_LEVEL_CRIT, __FILE__, __FUNCTION__, __LINE__, msglog); \
    }

#define LOG_TXT_ERR(fmt, args...) \
    { \
        LOG_MSG(fmt, ##args) \
        m_log.Write(MAINTENANCE_LOG_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__, msglog); \
    }

#define LOG_TXT_WARNING(fmt, args...) \
    { \
        LOG_MSG(fmt, ##args) \
        m_log.Write(MAINTENANCE_LOG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__, msglog); \
    }

#define LOG_TXT_NOTICE(fmt, args...) \
    { \
        LOG_MSG(fmt, ##args) \
        m_log.Write(MAINTENANCE_LOG_LEVEL_NOTICE, __FILE__, __FUNCTION__, __LINE__, msglog); \
    }

#define LOG_TXT_INFO(fmt, args...) \
    { \
        LOG_MSG(fmt, ##args) \
        m_log.Write(MAINTENANCE_LOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, msglog); \
    }

#define LOG_TXT_DEBUG(fmt, args...) \
    { \
        LOG_MSG(fmt, ##args) \
        m_log.Write(MAINTENANCE_LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, msglog); \
    }

#define LOG_TXT_NONE(fmt, args...) \
    { \
        LOG_MSG(fmt, ##args) \
        m_log.Write(MAINTENANCE_LOG_LEVEL_NONE, __FILE__, __FUNCTION__, __LINE__, msglog); \
    }

/* д������������־ */
#define LOG_BIN_PANIC(msg, msglen) \
    { \
        m_log.Write(MAINTENANCE_LOG_LEVEL_PANIC, __FILE__, __FUNCTION__, __LINE__, msg, msglen); \
    }

#define LOG_BIN_ALERT(msg, msglen) \
    { \
        m_log.Write(MAINTENANCE_LOG_LEVEL_ALERT, __FILE__, __FUNCTION__, __LINE__, msg, msglen); \
    }

#define LOG_BIN_CRIT(msg, msglen) \
    { \
        m_log.Write(MAINTENANCE_LOG_LEVEL_CRIT, __FILE__, __FUNCTION__, __LINE__, msg, msglen); \
    }

#define LOG_BIN_ERR(msg, msglen) \
    { \
        m_log.Write(MAINTENANCE_LOG_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__, msg, msglen); \
    }

#define LOG_BIN_WARNING(msg, msglen) \
    { \
        m_log.Write(MAINTENANCE_LOG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__, msg, msglen); \
    }

#define LOG_BIN_NOTICE(msg, msglen) \
    { \
        m_log.Write(MAINTENANCE_LOG_LEVEL_NOTICE, __FILE__, __FUNCTION__, __LINE__, msg, msglen); \
    }

#define LOG_BIN_INFO(msg, msglen) \
    { \
        m_log.Write(MAINTENANCE_LOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, msg, msglen); \
    }

#define LOG_BIN_DEBUG(msg, msglen) \
    { \
        m_log.Write(MAINTENANCE_LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, msg, msglen); \
    }

#define LOG_BIN_NONE(msg, msglen) \
    { \
        m_log.Write(MAINTENANCE_LOG_LEVEL_NONE, __FILE__, __FUNCTION__, __LINE__, msg, msglen); \
    }

#if 0 /* ϵͳ���÷��� */

int main()
{
    /* ��ʼ����־�ӿ� */
    LOG_INIT("../log/", "test", 10, 10, MAINTENANCE_LOG_LEVEL_NONE, "");

    /* ��־��� */
    int ret = 0;
    LOG_TXT_DEBUG("LOG_TXT_DEBUG[ret: %d]", ret);

    /* ��������־������� */
    LOG_BIN_DEBUG("1234567890", 10);

    return 0;
}

#endif

#endif
