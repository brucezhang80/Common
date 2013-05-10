/*
 * @File: cyclog.cpp
 * @Desc: impliment file of cyclog
 * @Author: stevetang
 * @History:
 *      2008-10-14   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "common.h"
#include "cyclog.h"

CCycLog::CCycLog(const char * logpath, const char * modname, int max_filesize, int max_filecnt) : 
	m_max_filesize(max_filesize * 1024 * 1024), m_max_filecnt(max_filecnt)
{
	/* 创建路径 */
	sprintf(m_logpath, "%s/%s", logpath, modname);
	CreateDirectory(m_logpath);

	sprintf(m_logfile, "%s/%s", m_logpath, MacDefCycLogFile);

	m_tlock = new CThreadLock();

	/* 创建文件锁 */
	char lockfile[MacMaxCycLogFileLen];

	sprintf(lockfile, "%s/%s", m_logpath, MacLckCycLogFile);

	m_lock = new CFileLock(lockfile);
}

CCycLog::~CCycLog()
{
	if (m_lock)
	{
		delete m_lock;
		m_lock = NULL;
	}

	if (m_tlock)
	{
		delete m_tlock;
		m_tlock = NULL;
	}
}

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
int CCycLog::CheckDir()
{
	DIR * dir = opendir(m_logpath);
	if (dir == NULL)
		return -1;

	int cnt = 0;
	char fname[MacMaxCycLogPathLen];

	/* 遍历目录取日志文件名 */
	memset(fname, 0, sizeof(fname));
	while (1)
	{
		struct dirent * dp = readdir(dir);
		if (dp == NULL)
			break;

		char * str = strchr(dp->d_name, '.');
		if (str == NULL)
			continue;

		if (strcmp(str, ".log") == 0)
		{
			if (strlen(fname) == 0 || strcmp(fname, dp->d_name) > 0)
			{
				/* 保留最小时间 */
				strcpy(fname, dp->d_name);
			}

			cnt++;
		}
	}

	closedir(dir);

	if (cnt > m_max_filecnt)
	{
		/* 超过文件数目 */
		char logfile[MacMaxCycLogFileLen];

		sprintf(logfile, "%s/%s", m_logpath, fname);

		remove(logfile); /* 删除文件 */
	}

	return 0;
}

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
int CCycLog::CheckFile()
{
	int ret = 0;

	/* 确认当前日志文件信息 */
	struct stat s;
	time_t t_now = time(NULL);
	struct tm t;
	localtime_r(&t_now, &t);

	ret = lstat(m_logfile, &s);
	if (ret == 0 &&                    /* 文件存在 */
		s.st_size < m_max_filesize)    /* 文件大小未超过 */
	{
		struct tm t_mod;
		localtime_r(&s.st_mtime, &t_mod);

		if (t.tm_year == t_mod.tm_year && 
			t.tm_mon == t_mod.tm_mon && 
			t.tm_mday == t_mod.tm_mday) /* 日期未变 */
		{
			/* 文件不需要变更 */
			return 0;
		}
	}

	/* 需要变更文件,创建新文件 */
	char logfile[MacMaxCycLogFileLen];
	struct timeval tp;
	
	gettimeofday(&tp, NULL);
	sprintf(logfile, 
			"%s/%04d%02d%02d%02d%02d%02d%03d.log", 
			m_logpath, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000));
	rename(m_logfile, logfile);

	/* 删除多余文件 */
	CheckDir();

	return 0;
}

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
int CCycLog::Write(const char * level, const char * file, const char * func, const int line, const char * msg)
{
	char tmp[MacMaxCycLogLineLen];
	time_t t_now = time(NULL);
	struct tm t;
	localtime_r((const time_t *)&t_now, &t);
	struct timeval tp;
	
	gettimeofday(&tp, NULL);

	/* 年月日时分秒毫秒 文件名 函数名 行号 信息  */
	snprintf(tmp, MacMaxCycLogLineLen-1, "[%s][%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%s][%d]%s\r\n", 
		level, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
		file, func, line, msg);

	CAutoLock tlock(m_tlock);
	CAutoLock lock(m_lock);

	/* 确认是否需要更新文件 */
	CheckFile();

	/* 写文件 */
	int fd = open(m_logfile, O_CREAT|O_APPEND|O_WRONLY, 0666);
	if (fd == -1)
	{
		return -1;
	}

	write(fd, tmp, strlen(tmp));

	close(fd);

	return 0;
}

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
int CCycLog::Write(const char * level, const char * file, const char * func, const int line, const char * msg, const int msglen)
{
	CAutoLock tlock(m_tlock);
	CAutoLock lock(m_lock);

	/* 确认是否需要更新文件 */
	CheckFile();

	char tmp[MacMaxCycLogLineLen];
	time_t t_now = time(NULL);
	struct tm t;
	localtime_r((const time_t *)&t_now, &t);
	struct timeval tp;
	
	gettimeofday(&tp, NULL);

	/* 年月日时分秒毫秒 文件名 函数名 行号 信息  */
	snprintf(tmp, MacMaxCycLogLineLen-1, "[%s][%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%s][%d]binary begin\r\n", 
		level, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
		file, func, line);

	/* 写文件 */
	int fd = open(m_logfile, O_CREAT|O_APPEND|O_WRONLY, 0666);
	if (fd == -1)
	{
		return -1;
	}

	write(fd, tmp, strlen(tmp));

	char binary[100];

	char * ptr = binary;
	for (int i=0; i<msglen; i++)
	{
		if (i && i%16 == 0) /* 换行 */
		{
			sprintf(tmp, "[%s][%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%s][%d]%s\r\n", 
				level, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
				file, func, line, binary);
			write(fd, tmp, strlen(tmp));

			ptr = binary;
			*ptr = 0x00;
		}

		sprintf(ptr, "%02X ", (unsigned char)msg[i]);
		ptr += 3;
	}

	if (strlen(binary))
	{
		sprintf(tmp, "[%s][%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%s][%d]%s\r\n", 
			level, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
			file, func, line, binary);
		write(fd, tmp, strlen(tmp));
	}

	snprintf(tmp, MacMaxCycLogLineLen-1, "[%s][%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%s][%d]binary end\r\n", 
		level, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
		file, func, line);
	write(fd, tmp, strlen(tmp));

	close(fd);

	return 0;
}
