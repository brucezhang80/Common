/*
 * @File: log.cpp
 * @Desc: impliment file of log
 * @Author: stevetang
 * @History:
 *      2008-10-16   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "cyclog.h"
#include "log.h"

#define LOG_CHECK(msg, level) \
	{ \
		if (strlen(m_filter)) \
		{ \
			char * p = m_filter; \
			unsigned char find = 0; \
			while (1) \
			{ \
				char * str = strchr(p, ','); \
				char filter[MacMaxFilterLen]; \
				if (str == NULL) \
				{ \
					strcpy(filter, p); \
				} \
				else \
				{ \
					strncpy(filter, p, str-p); \
					filter[str-p] = 0x00; \
				} \
				if (strstr(msg, filter)) \
				{ \
					find = 1; \
					break; \
				} \
				if (str == NULL) \
					break; \
				p = str + 1; \
			} \
			if (!find && m_level < level) \
			{ \
				return 0; \
			} \
		} \
		else if (m_level < level) \
		{ \
			return 0; \
		} \
	}

#define LEVEL_CHECK(level) \
	{ \
		if (m_level < level) \
		{ \
			return 0; \
		} \
	}

CLog m_log;

void * CLog::m_pCycLog = NULL;

const char * MAINTENANCE_LOG_LEVEL_MSG[] = {
	"PANIC", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG", "NONE"
};

CLog::CLog() : m_level(MAINTENANCE_LOG_LEVEL_NONE)
{
	m_filter[0] = 0x00;
}

CLog::~CLog()
{
	if (m_pCycLog)
	{
		CCycLog * log = (CCycLog *)m_pCycLog;

		delete log;

		m_pCycLog = NULL;
	}
}

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
int CLog::Init(const char * logpath, const char * modname, int max_filesize, int max_filecnt, int level /* = MAINTENANCE_LOG_LEVEL_NONE */, const char * filter /* = "" */)
{
	if (m_pCycLog)
	{
		/* �ѳ�ʼ�� */
		return 0;
	}

	CCycLog * log = new CCycLog(logpath, modname, max_filesize, max_filecnt);
	if (log == NULL)
		return -1;

	m_pCycLog = log;
	m_level = level;

	strcpy(m_filter, filter);

	return 0;
}

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
int CLog::Write(int level, const char * file, const char * func, const int line, const char * msg)
{
	if (m_pCycLog == NULL)
		return -1;

	/* ������ */
	LOG_CHECK(msg, level);

	CCycLog * log = (CCycLog *)m_pCycLog;

	return log->Write(MAINTENANCE_LOG_LEVEL_MSG[level], file, func, line, msg);
}

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
int CLog::Write(int level, const char * file, const char * func, const int line, const char * msg, const int msglen)
{
	if (m_pCycLog == NULL)
		return -1;

	/* ������ */
	LEVEL_CHECK(level);

	CCycLog * log = (CCycLog *)m_pCycLog;

	return log->Write(MAINTENANCE_LOG_LEVEL_MSG[level], file, func, line, msg, msglen);
}
