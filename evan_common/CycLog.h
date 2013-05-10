/*
 * CycLog.h
 *
 *  Created on: 2010-4-26
 *      Author: evantang
 */

#ifndef CYCLOG_H_
#define CYCLOG_H_

#include "lock.h"
#include "general.h"
#include "common.h"

#define MacMaxCycLogPathLen    128
#define MacMaxCycLogNameLen    50
#define MacMaxCycLogSuffixLen  10
#define MacMaxCycLogFileLen    512
#define MacMaxCycLogLineLen    8192
#define MacDefCycLogFile       "tmp.log_"
#define MacLckCycLogFile       "log.lck_"

class CCycFile
{
protected:
	char m_logpath[MacMaxCycLogPathLen]; /* 日志路径 */
	char m_logfile[MacMaxCycLogFileLen]; /* 当前日志文件 */
	char m_suffix[MacMaxCycLogSuffixLen];/* 文件名后缀 */
	char m_prefix[MacMaxCycLogSuffixLen];/* 文件名前缀 */

	int m_max_filesize; /* 最大文件大小 */
	int m_max_filecnt; /* 最多的文件数目 */

	CFileLock * m_lock;

public:
	CCycFile(const char * logpath, const char * modname, const char* prefix, const char* suffix, int max_filesize, int max_filecnt) :
		m_max_filesize(max_filesize * 1024 * 1024), m_max_filecnt(max_filecnt)
	{
		/* 创建路径 */
		memset(m_logpath, 0, MacMaxCycLogPathLen);
		sprintf(m_logpath, "%s/%s", logpath, modname);
		CreateDirectory(m_logpath);

		memset(m_logfile, 0, MacMaxCycLogFileLen);
		sprintf(m_logfile, "%s/%s", m_logpath, MacDefCycLogFile);

		memset(m_suffix, 0, MacMaxCycLogSuffixLen);
		sprintf(m_suffix, ".%s", suffix);

		memset(m_prefix, 0, MacMaxCycLogSuffixLen);
		sprintf(m_prefix, "%s", prefix);

		/* 创建文件锁 */
		char lockfile[MacMaxCycLogFileLen];

		sprintf(lockfile, "%s/%s", m_logpath, MacLckCycLogFile);

		m_lock = new CFileLock(lockfile);
	}

	virtual ~CCycFile()
	{
		if (m_lock)
		{
			delete m_lock;
			m_lock = NULL;
		}
	}

protected:
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
	int CheckDir()
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

			if (strcmp(str, m_suffix) == 0)
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
	 * CheckFileDate 虚函数，判断文件时间和当前时间的区别，确定什么时候文件改名
	 *
	 *
	 *
	 */
	virtual bool CheckFileDate(const struct tm* t, const struct tm* t_mod) = 0;
#if 0
	{
		if (t.tm_year == t_mod.tm_year &&
			t.tm_mon == t_mod.tm_mon &&
			t.tm_mday == t_mod.tm_mday) /* 日期未变 */
		{
			/* 文件不需要变更 */
			return true;
		}
		else
			return false;
	}
#endif

	virtual void SetLogFileName(char* logfile, const struct tm* t) = 0;
#if 0
	{
		struct timeval tp;

		gettimeofday(&tp, NULL);
		sprintf(logfile,
				"%s/%04d%02d%02d%02d%02d%02d%03d.log",
				m_logpath, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min,
				t.tm_sec, (int)(tp.tv_usec/1000));
	}
#endif

public:
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
	int CheckFile()
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

			if (CheckFileDate(&t, &t_mod) == true) /* 日期未变 */
			{
				/* 文件不需要变更 */
				return 0;
			}
		}

		/* 需要变更文件,创建新文件 */
		char logfile[MacMaxCycLogFileLen] = {0};
		SetLogFileName(logfile, &t);
		rename(m_logfile, logfile);

		/* 删除多余文件 */
		CheckDir();

		return 0;
	}
    /*
     * Function: Write
     * Desc: 写文本日志
     * In:
     *      const char *   buf 内容缓冲
     *      const char *   buf 内容长度
     * Out:
     *      none
     * Return code:
     *      0  -   成功
     *      -1 -   失败
     */
    int Write(const char* buf, const unsigned int len)
    {
    	CAutoLock lock(m_lock);

    	/* 确认是否需要更新文件 */
    	CheckFile();

    	/* 写文件 */
    	int fd = open(m_logfile, O_CREAT|O_APPEND|O_WRONLY, 0666);
    	if (fd == -1)
    	{
    		return -1;
    	}

    	write(fd, buf, len);

    	close(fd);

    	return 0;
    }
};

#endif /* CYCLOG_H_ */
