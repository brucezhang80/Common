/*
 * @File: lock.h
 * @Desc: head file of lock
 * @Author: stevetang geminiguan xiahz paul
 * @History:
 *      2007-05-16   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _LOCK_H
#define _LOCK_H

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class CLock {
public:
    CLock() { m_islock = 0; };
    virtual ~CLock() { m_islock = 0; };

protected:
    unsigned char m_islock; /* �Ƿ��Ѽ��� 0:�� 1:�� */

protected:
    /*
     * Function: IsLock
     * Desc: �Ƿ��Ѽ���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    bool IsLock() { return (m_islock == 1); };

public:
    /*
     * Function: Lock
     * Desc: ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    virtual int Lock() { m_islock = 1; return 0; };

    /*
     * Function: UnLock
     * Desc: �ͷ���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    virtual int UnLock() { m_islock = 0; return 0; };
};

class CLockEx {
    friend class CAutoLockEx;

public:
    CLockEx() { m_cnt = 0; };
    virtual ~CLockEx() { m_cnt = 0; };

protected:
    int m_cnt; /* lock����Ŀ */

public:
    /*
     * Function: Lock
     * Desc: ����
     * In: 
     *     int  pos       λ��
     *     int  cnt       ��Ŀ
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    virtual int Lock(int pos = 0, int cnt = 0) { m_cnt++; return 0; };

    /*
     * Function: UnLock
     * Desc: �ͷ���
     * In: 
     *     int  pos       λ��
     *     int  cnt       ��Ŀ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    virtual int UnLock(int pos = 0, int cnt = 0) { m_cnt--; return 0; };
};

/* �߳� */
class CThreadLock : public CLock {
#ifdef WIN32
	HANDLE m_mutex; /* ������ */
#else
    pthread_mutex_t  m_mutex;  /* ������ */
#endif

public:
    CThreadLock();
    ~CThreadLock();

public:
    /*
     * Function: Lock
     * Desc: ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Lock();

    /*
     * Function: UnLock
     * Desc: �ͷ���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int UnLock();
};

#ifdef WIN32
#else
/* ���� */
class CSemLock : public CLock {
    int m_semid;  /* �ź��� */

public:
    CSemLock(int key);
    ~CSemLock();

private:
    /*
     * Function: Reset
     * Desc: ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Reset();

public:
    /*
     * Function: Lock
     * Desc: ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Lock();

    /*
     * Function: UnLock
     * Desc: �ͷ���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int UnLock();

    /*
     * Function: GetValue
     * Desc: ȡ�ź���ֵ
     * In: 
     *     int  pos       λ��
     *     int  cnt       ��Ŀ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int GetValue(unsigned short * value);
};

/* ���ź��� */
class CSemLockEx : public CLockEx {
    int m_semid;  /* �ź��� */
    int m_num; /* �ź�����Ŀ */

public:
    CSemLockEx(int key, int num);
    ~CSemLockEx();

private:
    /*
     * Function: Reset
     * Desc: ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Reset();

public:
    /*
     * Function: Lock
     * Desc: ����
     * In: 
     *     int  pos       λ��
     *     int  cnt       ��Ŀ
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Lock(int pos, int cnt);

    /*
     * Function: UnLock
     * Desc: �ͷ���
     * In: 
     *     int  pos       λ��
     *     int  cnt       ��Ŀ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int UnLock(int pos, int cnt);

    /*
     * Function: GetValue
     * Desc: ȡ�ź���ֵ
     * In: 
     *     int  pos       λ��
     *     int  cnt       ��Ŀ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int GetValue(unsigned short * value, int cnt);
};

/* �ļ��� */
class CFileLock : public CLock {
    int m_fd; /* �ļ���� */

public:
    CFileLock(const char * fname);
    CFileLock(int fd);
    ~CFileLock();

public:
    /*
     * Function: Lock
     * Desc: ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Lock();

    /*
     * Function: UnLock
     * Desc: �ͷ���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int UnLock();
};

/* �ļ��� */
class CFileLockEx : public CLockEx {
    int m_fd; /* �ļ���� */
    short m_type; /* ������ */

public:
    CFileLockEx(const char * fname, short ltype);
    CFileLockEx(int fd, short ltype);
    ~CFileLockEx();

public:
    /*
     * Function: Lock
     * Desc: ����
     * In: 
     *     int  pos       λ��
     *     int  cnt       ��Ŀ
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Lock(int pos, int cnt);

    /*
     * Function: UnLock
     * Desc: �ͷ���
     * In: 
     *     int  pos       λ��
     *     int  cnt       ��Ŀ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int UnLock(int pos, int cnt);
};

/* �ڰ��� */
class CIDLockEx : public CLockEx {
    int m_num;
    pthread_mutex_t  m_mutex;
    int * m_lockid;
    int * m_waitnum;
    int * m_lockflag;
    pthread_cond_t * m_wqueue;

public:
    CIDLockEx(int num);
    ~CIDLockEx();

public:
    /*
     * Function: Lock
     * Desc: ����
     * In: 
     *     int  lockId          ��������
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Lock(int lockId, int cnt);

    /*
     * Function: UnLock
     * Desc: �ͷ���
     * In: 
     *     int  lockId          ��������
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int UnLock(int lockId, int cnt);
};
#endif

class CAutoLock {
    CLock * m_lock;

public:
    CAutoLock(CLock * l) { 
        m_lock = l;
        m_lock->Lock(); 
    };

    ~CAutoLock() { 
        m_lock->UnLock(); 
    };
};

class CAutoLockEx {
    CLockEx * m_lock;
    int m_lockId;
    int m_lockCnt;

public:
    CAutoLockEx(CLockEx * l, int lockId, int lockCnt = 1) { 
        m_lock = l;
        m_lockId = lockId;
        m_lockCnt = lockCnt;
        m_lock->Lock(m_lockId, m_lockCnt); 
    };

    ~CAutoLockEx() {
        m_lock->UnLock(m_lockId, m_lockCnt); 
    };
};

#endif
