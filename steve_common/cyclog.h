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
	char m_logpath[MacMaxCycLogPathLen]; /* ��־·�� */
	char m_logfile[MacMaxCycLogFileLen]; /* ��ǰ��־�ļ� */

	int m_max_filesize; /* ����ļ���С */
	int m_max_filecnt; /* �����ļ���Ŀ */

	CThreadLock * m_tlock; /* �����߳� */
	CFileLock * m_lock; /* ���ڽ��� */

public:
	CCycLog(const char * logpath, const char * modname, int max_filesize, int max_filecnt);
	~CCycLog();

private:
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
	int CheckDir();

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
	int CheckFile();

public:
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
    int Write(const char * level, const char * file, const char * func, const int line, const char * msg);

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
    int Write(const char * level, const char * file, const char * func, const int line, const char * msg, const int msglen);
};

#endif
