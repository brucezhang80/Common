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
 * Desc: 初始化
 * In: 
 *      const char *   logpath      文件名
 *      const char *   modname      函数名
 *      int			   max_filesize 最大文件长度
 *      int			   max_filecnt  最多文件数目
 *      int            level        日志级别
 *      const char *   filter       过滤串
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CLog::Init(const char * logpath, const char * modname, int max_filesize, int max_filecnt, int level /* = MAINTENANCE_LOG_LEVEL_NONE */, const char * filter /* = "" */)
{
	if (m_pCycLog)
	{
		/* 已初始化 */
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
 * Desc: 写文本日志
 * In: 
 *      int            level     日志级别
 *      const char *   file      文件名
 *      const char *   func      函数名
 *      const int      line      行号
 *      const char *   msg       日志信息
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CLog::Write(int level, const char * file, const char * func, const int line, const char * msg)
{
	if (m_pCycLog == NULL)
		return -1;

	/* 级别检查 */
	LOG_CHECK(msg, level);

	CCycLog * log = (CCycLog *)m_pCycLog;

	return log->Write(MAINTENANCE_LOG_LEVEL_MSG[level], file, func, line, msg);
}

/*
 * Function: Write
 * Desc: 写二进制日志
 * In: 
 *      int            level     日志级别
 *      const char *   file      文件名
 *      const char *   func      函数名
 *      const int      line      行号
 *      const char *   msg       日志信息
 *      const int      msglen    信息长度
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CLog::Write(int level, const char * file, const char * func, const int line, const char * msg, const int msglen)
{
	if (m_pCycLog == NULL)
		return -1;

	/* 级别检查 */
	LEVEL_CHECK(level);

	CCycLog * log = (CCycLog *)m_pCycLog;

	return log->Write(MAINTENANCE_LOG_LEVEL_MSG[level], file, func, line, msg, msglen);
}
