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
	char m_logpath[MacMaxCycLogPathLen]; /* ��־·�� */
	char m_logfile[MacMaxCycLogFileLen]; /* ��ǰ��־�ļ� */
	char m_suffix[MacMaxCycLogSuffixLen];/* �ļ�����׺ */
	char m_prefix[MacMaxCycLogSuffixLen];/* �ļ���ǰ׺ */

	int m_max_filesize; /* ����ļ���С */
	int m_max_filecnt; /* �����ļ���Ŀ */

	CFileLock * m_lock;

public:
	CCycFile(const char * logpath, const char * modname, const char* prefix, const char* suffix, int max_filesize, int max_filecnt) :
		m_max_filesize(max_filesize * 1024 * 1024), m_max_filecnt(max_filecnt)
	{
		/* ����·�� */
		memset(m_logpath, 0, MacMaxCycLogPathLen);
		sprintf(m_logpath, "%s/%s", logpath, modname);
		CreateDirectory(m_logpath);

		memset(m_logfile, 0, MacMaxCycLogFileLen);
		sprintf(m_logfile, "%s/%s", m_logpath, MacDefCycLogFile);

		memset(m_suffix, 0, MacMaxCycLogSuffixLen);
		sprintf(m_suffix, ".%s", suffix);

		memset(m_prefix, 0, MacMaxCycLogSuffixLen);
		sprintf(m_prefix, "%s", prefix);

		/* �����ļ��� */
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
     * Desc: ���Ŀ¼���ļ����Ƿ��ѵ�
     * In:
     *      none
     * Out:
     *      none
     * Return code:
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int CheckDir()
	{
		DIR * dir = opendir(m_logpath);
		if (dir == NULL)
			return -1;

		int cnt = 0;
		char fname[MacMaxCycLogPathLen];

		/* ����Ŀ¼ȡ��־�ļ��� */
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
					/* ������Сʱ�� */
					strcpy(fname, dp->d_name);
				}

				cnt++;
			}
		}

		closedir(dir);

		if (cnt > m_max_filecnt)
		{
			/* �����ļ���Ŀ */
			char logfile[MacMaxCycLogFileLen];

			sprintf(logfile, "%s/%s", m_logpath, fname);

			remove(logfile); /* ɾ���ļ� */
		}

		return 0;
	}

	/*
	 * CheckFileDate �麯�����ж��ļ�ʱ��͵�ǰʱ�������ȷ��ʲôʱ���ļ�����
	 *
	 *
	 *
	 */
	virtual bool CheckFileDate(const struct tm* t, const struct tm* t_mod) = 0;
#if 0
	{
		if (t.tm_year == t_mod.tm_year &&
			t.tm_mon == t_mod.tm_mon &&
			t.tm_mday == t_mod.tm_mday) /* ����δ�� */
		{
			/* �ļ�����Ҫ��� */
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
     * Desc: ����Ƿ���Ҫ�����ļ���
     * In:
     *      none
     * Out:
     *      none
     * Return code:
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int CheckFile()
	{
		int ret = 0;

		/* ȷ�ϵ�ǰ��־�ļ���Ϣ */
		struct stat s;
		time_t t_now = time(NULL);
		struct tm t;
		localtime_r(&t_now, &t);

		ret = lstat(m_logfile, &s);
		if (ret == 0 &&                    /* �ļ����� */
			s.st_size < m_max_filesize)    /* �ļ���Сδ���� */
		{
			struct tm t_mod;
			localtime_r(&s.st_mtime, &t_mod);

			if (CheckFileDate(&t, &t_mod) == true) /* ����δ�� */
			{
				/* �ļ�����Ҫ��� */
				return 0;
			}
		}

		/* ��Ҫ����ļ�,�������ļ� */
		char logfile[MacMaxCycLogFileLen] = {0};
		SetLogFileName(logfile, &t);
		rename(m_logfile, logfile);

		/* ɾ�������ļ� */
		CheckDir();

		return 0;
	}
    /*
     * Function: Write
     * Desc: д�ı���־
     * In:
     *      const char *   buf ���ݻ���
     *      const char *   buf ���ݳ���
     * Out:
     *      none
     * Return code:
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    int Write(const char* buf, const unsigned int len)
    {
    	CAutoLock lock(m_lock);

    	/* ȷ���Ƿ���Ҫ�����ļ� */
    	CheckFile();

    	/* д�ļ� */
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
