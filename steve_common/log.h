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

/***********************************************日志级别定义*********************************************************/
//值	|级别		|描述								|文本类存储时长（可以动态配置）		|监控类日志建议级别
//0		|PANIC		|该系统不可用						|3年					|
//1		|ALERT		|需要立即被修改的条件				|2年					|
//2		|CRIT		|阻止某些工具或子系统功能实现的错误
//                  |条件				                |1年					|
//3		|ERR		|阻止工具或某些子系统部分功能实现的
//                  |错误条件			                |6个月					|异常类
//4		|WARNING	|预警信息							|3个月					|逻辑错误类
//5		|NOTICE		|具有重要性的普通条件				|2个月					|普通状态类
//6		|INFO		|提供信息的消息						|1个月					|默认
//7		|DEBUG		|不包含函数条件或问题的其他信息		|15天					|
//8		|NONE		|没有重要级，通常用于排错			|1天					|

#ifndef MAINTENANCE_LOG_LEVEL_PANIC
#define MAINTENANCE_LOG_LEVEL_PANIC	0	//该系统不可用
#endif

#ifndef MAINTENANCE_LOG_LEVEL_ALERT
#define MAINTENANCE_LOG_LEVEL_ALERT	1	//需要立即被修改的条件
#endif

#ifndef MAINTENANCE_LOG_LEVEL_CRIT
#define MAINTENANCE_LOG_LEVEL_CRIT	2	//阻止某些工具或子系统功能实现的错误条件
#endif

#ifndef MAINTENANCE_LOG_LEVEL_ERR
#define MAINTENANCE_LOG_LEVEL_ERR	3	//阻止工具或某些子系统部分功能实现的错误条件
#endif

#ifndef MAINTENANCE_LOG_LEVEL_WARNING
#define MAINTENANCE_LOG_LEVEL_WARNING	4	//预警信息
#endif

#ifndef MAINTENANCE_LOG_LEVEL_NOTICE
#define MAINTENANCE_LOG_LEVEL_NOTICE	5	//具有重要性的普通条件
#endif

#ifndef MAINTENANCE_LOG_LEVEL_INFO
#define MAINTENANCE_LOG_LEVEL_INFO	6	//提供信息的消息
#endif

#ifndef MAINTENANCE_LOG_LEVEL_DEBUG
#define MAINTENANCE_LOG_LEVEL_DEBUG	7	//不包含函数条件或问题的其他信息
#endif

#ifndef MAINTENANCE_LOG_LEVEL_NONE
#define MAINTENANCE_LOG_LEVEL_NONE	8	//没有重要级，通常用于排错
#endif

#define MacMaxFilterLen     1024

class CLog
{
	int m_level; /* 日志级别 */
	char m_filter[MacMaxFilterLen]; /* Filter串 */
	static void * m_pCycLog; /* 日志对象 */

public:
	CLog();
	~CLog();

public:
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
	int Init(const char * logpath, const char * modname, int max_filesize, int max_filecnt, int level = MAINTENANCE_LOG_LEVEL_NONE, const char * filter = "");

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
    int Write(int level, const char * file, const char * func, const int line, const char * msg);

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
    int Write(int level, const char * file, const char * func, const int line, const char * msg, const int msglen);
};

extern CLog m_log;

#define MAX_LOG_MSGLEN      8192

#define LOG_MSG(fmt, args...) \
            char msglog[MAX_LOG_MSGLEN]; \
            snprintf(msglog, MAX_LOG_MSGLEN-1, fmt, ##args);

/* 初始化日志 */
#define LOG_INIT(logpath, modname, max_filesize, max_filecnt, args...) \
    { \
        m_log.Init(logpath, modname, max_filesize, max_filecnt, ##args); \
    }

/* 写文本日志 */
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

/* 写二进制数据日志 */
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

#if 0 /* 系统调用范例 */

int main()
{
    /* 初始化日志接口 */
    LOG_INIT("../log/", "test", 10, 10, MAINTENANCE_LOG_LEVEL_NONE, "");

    /* 日志输出 */
    int ret = 0;
    LOG_TXT_DEBUG("LOG_TXT_DEBUG[ret: %d]", ret);

    /* 二进制日志数据输出 */
    LOG_BIN_DEBUG("1234567890", 10);

    return 0;
}

#endif

#endif
