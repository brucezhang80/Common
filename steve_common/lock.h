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
    unsigned char m_islock; /* 是否已加锁 0:否 1:是 */

protected:
    /*
     * Function: IsLock
     * Desc: 是否已加锁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    bool IsLock() { return (m_islock == 1); };

public:
    /*
     * Function: Lock
     * Desc: 加锁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    virtual int Lock() { m_islock = 1; return 0; };

    /*
     * Function: UnLock
     * Desc: 释放锁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    virtual int UnLock() { m_islock = 0; return 0; };
};

class CLockEx {
    friend class CAutoLockEx;

public:
    CLockEx() { m_cnt = 0; };
    virtual ~CLockEx() { m_cnt = 0; };

protected:
    int m_cnt; /* lock的数目 */

public:
    /*
     * Function: Lock
     * Desc: 加锁
     * In: 
     *     int  pos       位置
     *     int  cnt       数目
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    virtual int Lock(int pos = 0, int cnt = 0) { m_cnt++; return 0; };

    /*
     * Function: UnLock
     * Desc: 释放锁
     * In: 
     *     int  pos       位置
     *     int  cnt       数目
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    virtual int UnLock(int pos = 0, int cnt = 0) { m_cnt--; return 0; };
};

/* 线程 */
class CThreadLock : public CLock {
#ifdef WIN32
	HANDLE m_mutex; /* 互斥锁 */
#else
    pthread_mutex_t  m_mutex;  /* 互斥锁 */
#endif

public:
    CThreadLock();
    ~CThreadLock();

public:
    /*
     * Function: Lock
     * Desc: 加锁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    int Lock();

    /*
     * Function: UnLock
     * Desc: 释放锁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int UnLock();
};

#ifdef WIN32
#else
/* 进程 */
class CSemLock : public CLock {
    int m_semid;  /* 信号量 */

public:
    CSemLock(int key);
    ~CSemLock();

private:
    /*
     * Function: Reset
     * Desc: 重置
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    int Reset();

public:
    /*
     * Function: Lock
     * Desc: 加锁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    int Lock();

    /*
     * Function: UnLock
     * Desc: 释放锁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int UnLock();

    /*
     * Function: GetValue
     * Desc: 取信号量值
     * In: 
     *     int  pos       位置
     *     int  cnt       数目
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int GetValue(unsigned short * value);
};

/* 多信号量 */
class CSemLockEx : public CLockEx {
    int m_semid;  /* 信号量 */
    int m_num; /* 信号量数目 */

public:
    CSemLockEx(int key, int num);
    ~CSemLockEx();

private:
    /*
     * Function: Reset
     * Desc: 重置
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    int Reset();

public:
    /*
     * Function: Lock
     * Desc: 加锁
     * In: 
     *     int  pos       位置
     *     int  cnt       数目
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    int Lock(int pos, int cnt);

    /*
     * Function: UnLock
     * Desc: 释放锁
     * In: 
     *     int  pos       位置
     *     int  cnt       数目
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int UnLock(int pos, int cnt);

    /*
     * Function: GetValue
     * Desc: 取信号量值
     * In: 
     *     int  pos       位置
     *     int  cnt       数目
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int GetValue(unsigned short * value, int cnt);
};

/* 文件锁 */
class CFileLock : public CLock {
    int m_fd; /* 文件句柄 */

public:
    CFileLock(const char * fname);
    CFileLock(int fd);
    ~CFileLock();

public:
    /*
     * Function: Lock
     * Desc: 加锁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    int Lock();

    /*
     * Function: UnLock
     * Desc: 释放锁
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int UnLock();
};

/* 文件锁 */
class CFileLockEx : public CLockEx {
    int m_fd; /* 文件句柄 */
    short m_type; /* 锁类型 */

public:
    CFileLockEx(const char * fname, short ltype);
    CFileLockEx(int fd, short ltype);
    ~CFileLockEx();

public:
    /*
     * Function: Lock
     * Desc: 加锁
     * In: 
     *     int  pos       位置
     *     int  cnt       数目
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    int Lock(int pos, int cnt);

    /*
     * Function: UnLock
     * Desc: 释放锁
     * In: 
     *     int  pos       位置
     *     int  cnt       数目
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int UnLock(int pos, int cnt);
};

/* 黑白锁 */
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
     * Desc: 加锁
     * In: 
     *     int  lockId          被锁数据
     * Out: 
     *     none
     * Return code:
     *     -1                   出错
     *      0                   成功
     */
    int Lock(int lockId, int cnt);

    /*
     * Function: UnLock
     * Desc: 释放锁
     * In: 
     *     int  lockId          被锁数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
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
