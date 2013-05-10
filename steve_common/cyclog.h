/*
 * @File: cyclog.h
 * @Desc: head file of cyclog
 * @Author: stevetang
 * @History:
 *      2008-10-14   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _CYCLOG_H
#define _CYCLOG_H

#include "lock.h"

#define MacMaxCycLogPathLen    128
#define MacMaxCycLogNameLen    50
#define MacMaxCycLogFileLen    512
#define MacMaxCycLogLineLen    8192
#define MacDefCycLogFile       "tmp.log_"
#define MacLckCycLogFile       "log.lck_"

class CCycLog
{
	char m_logpath[MacMaxCycLogPathLen]; /* 日志路径 */
	char m_logfile[MacMaxCycLogFileLen]; /* 当前日志文件 */

	int m_max_filesize; /* 最大文件大小 */
	int m_max_filecnt; /* 最多的文件数目 */

	CThreadLock * m_tlock; /* 用于线程 */
	CFileLock * m_lock; /* 用于进程 */

public:
	CCycLog(const char * logpath, const char * modname, int max_filesize, int max_filecnt);
	~CCycLog();

private:
    /*
     * Function: CheckDir
     * Desc: 检查目录下文件数是否已到
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int CheckDir();

    /*
     * Function: CheckFile
     * Desc: 检查是否需要更换文件名
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	int CheckFile();

public:
    /*
     * Function: Write
     * Desc: 写文本日志
     * In: 
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
    int Write(const char * level, const char * file, const char * func, const int line, const char * msg);

    /*
     * Function: Write
     * Desc: 写二进制日志
     * In: 
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
    int Write(const char * level, const char * file, const char * func, const int line, const char * msg, const int msglen);
};

#endif
