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
	/* ����·�� */
	sprintf(m_logpath, "%s/%s", logpath, modname);
	CreateDirectory(m_logpath);

	sprintf(m_logfile, "%s/%s", m_logpath, MacDefCycLogFile);

	m_tlock = new CThreadLock();

	/* �����ļ��� */
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
 * Desc: ���Ŀ¼���ļ����Ƿ��ѵ�
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CCycLog::CheckDir()
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

		if (strcmp(str, ".log") == 0)
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
int CCycLog::CheckFile()
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

		if (t.tm_year == t_mod.tm_year && 
			t.tm_mon == t_mod.tm_mon && 
			t.tm_mday == t_mod.tm_mday) /* ����δ�� */
		{
			/* �ļ�����Ҫ��� */
			return 0;
		}
	}

	/* ��Ҫ����ļ�,�������ļ� */
	char logfile[MacMaxCycLogFileLen];
	struct timeval tp;
	
	gettimeofday(&tp, NULL);
	sprintf(logfile, 
			"%s/%04d%02d%02d%02d%02d%02d%03d.log", 
			m_logpath, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000));
	rename(m_logfile, logfile);

	/* ɾ�������ļ� */
	CheckDir();

	return 0;
}

/*
 * Function: Write
 * Desc: д�ı���־
 * In: 
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
int CCycLog::Write(const char * level, const char * file, const char * func, const int line, const char * msg)
{
	char tmp[MacMaxCycLogLineLen];
	time_t t_now = time(NULL);
	struct tm t;
	localtime_r((const time_t *)&t_now, &t);
	struct timeval tp;
	
	gettimeofday(&tp, NULL);

	/* ������ʱ������� �ļ��� ������ �к� ��Ϣ  */
	snprintf(tmp, MacMaxCycLogLineLen-1, "[%s][%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%s][%d]%s\r\n", 
		level, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
		file, func, line, msg);

	CAutoLock tlock(m_tlock);
	CAutoLock lock(m_lock);

	/* ȷ���Ƿ���Ҫ�����ļ� */
	CheckFile();

	/* д�ļ� */
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
 * Desc: д��������־
 * In: 
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
int CCycLog::Write(const char * level, const char * file, const char * func, const int line, const char * msg, const int msglen)
{
	CAutoLock tlock(m_tlock);
	CAutoLock lock(m_lock);

	/* ȷ���Ƿ���Ҫ�����ļ� */
	CheckFile();

	char tmp[MacMaxCycLogLineLen];
	time_t t_now = time(NULL);
	struct tm t;
	localtime_r((const time_t *)&t_now, &t);
	struct timeval tp;
	
	gettimeofday(&tp, NULL);

	/* ������ʱ������� �ļ��� ������ �к� ��Ϣ  */
	snprintf(tmp, MacMaxCycLogLineLen-1, "[%s][%04d-%02d-%02d %02d:%02d:%02d:%03d][%s][%s][%d]binary begin\r\n", 
		level, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (int)(tp.tv_usec/1000), 
		file, func, line);

	/* д�ļ� */
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
		if (i && i%16 == 0) /* ���� */
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
