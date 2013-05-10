/*
 * @File: lock.cpp
 * @Desc: implement file of lock
 * @Author: stevetang
 * @History:
 *      2007-05-16   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */


#ifdef WIN32
#else
#include <sys/file.h>

#include <sys/ipc.h>
#include<sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/sem.h>
#include <string>
#endif

#include "lock.h"

CThreadLock::CThreadLock() 
{
    /* 初始化互斥锁 */
#ifdef WIN32
	m_mutex = CreateMutex(NULL, FALSE, NULL);
#else
    pthread_mutex_init(&m_mutex, NULL);
#endif
}

CThreadLock::~CThreadLock()
{
    /* 释放互斥锁 */
#ifdef WIN32
	CloseHandle(m_mutex);
#else
    pthread_mutex_destroy(&m_mutex);
#endif
}

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
int CThreadLock::Lock()
{
    /* 加锁 */
#ifdef WIN32
	int ret = WaitForSingleObject(m_mutex, INFINITE);
	if (ret != WAIT_OBJECT_0)
	{
		return -1;
	}
#else
    int ret = pthread_mutex_lock(&m_mutex);
	if (ret)
	{
		return -1;
	}
#endif

    return CLock::Lock();
}

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
int CThreadLock::UnLock()
{
    /* 释放锁 */
#ifdef WIN32
	int ret = ReleaseMutex(m_mutex);
	if (!ret)
	{
		return -1;
	}
#else
    int ret = pthread_mutex_unlock(&m_mutex);
    if (ret)
    {
        return -1;
    }
#endif

    return CLock::UnLock();
}

#ifdef WIN32
#else
CSemLock::CSemLock(int key) : m_semid(-1)
{
    /* 初始化信号量 */
	m_semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
	if (m_semid < 0) { 
		if (errno != EEXIST) {
			return;
		}
		m_semid = semget(key, 1, 0666);
		if( m_semid < 0 ) {
			return;
		}
	}
	else
	{
		Reset();
	}
}

CSemLock::~CSemLock()
{
    if (m_semid > 0)
    {
        /* 释放互斥锁 */
        close(m_semid);
        m_semid = -1;
    }
}

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
int CSemLock::Reset()
{
    if (m_semid == -1)
    {
        return -1;
    }

    // init sem
	unsigned short* init_array = new unsigned short[1];
	init_array[0] = 1;
	int ret = semctl(m_semid, 0, SETALL, init_array);
	delete [] init_array;
	if(ret < 0) {
		return -1;
	}

    return 0;
}

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
int CSemLock::Lock()
{
    if (m_semid == -1)
    {
        return -1;
    }

    /* 加锁 */
    for(;;) {
        struct sembuf sops;
        sops.sem_num = 0;
        sops.sem_op = -1;
        sops.sem_flg = SEM_UNDO;

        int ret = semop(m_semid, &sops, 1);
        if(ret<0) {
			if(errno == EINTR) {
				continue;
			}
			else {
                return -1;
			}
        } else {
            break;
        }
    }

    return CLock::Lock();
}

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
int CSemLock::UnLock()
{
    if (m_semid == -1)
    {
        return -1;
    }

    /* 释放锁 */
    for(;;) {
        struct sembuf sops;
        sops.sem_num = 0;
        sops.sem_op = 1;
        sops.sem_flg = SEM_UNDO;

        int ret = semop(m_semid, &sops, 1);
        if(ret<0) {
			if(errno == EINTR) {
				continue;
			}
			else {
                return -1;
			}
        } else {
            break;
        }
    }

    return CLock::UnLock();
}

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
int CSemLock::GetValue(unsigned short * value)
{
	int ret = semctl(m_semid, 1, GETALL, value);
	if(ret < 0) {
		return -1;
	}

	return 0;
}

CSemLockEx::CSemLockEx(int key, int num) : m_semid(-1), m_num(num)
{
    /* 初始化信号量 */
	m_semid = semget(key, m_num, IPC_CREAT | IPC_EXCL | 0666);
	if (m_semid < 0) { 
		if (errno != EEXIST) {
			return;
		}
		m_semid = semget(key, m_num, 0666);
		if( m_semid < 0 ) {
			return;
		}
	}
	else
    {
        /* 重置 */
        Reset();
    }
}

CSemLockEx::~CSemLockEx()
{
    if (m_semid > 0)
    {
        /* 释放互斥锁 */
        close(m_semid);
        m_semid = -1;
    }
}

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
int CSemLockEx::Reset()
{
    if (m_semid == -1)
    {
        return -1;
    }

    // init sem
	unsigned short* init_array = new unsigned short[m_num];

    for (int i=0; i<m_num; i++)
        init_array[i] = 1;

	int ret = semctl(m_semid, m_num, SETALL, init_array);
	delete [] init_array;
	if(ret < 0) {
		return -1;
	}

    return 0;
}

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
int CSemLockEx::Lock(int pos, int cnt)
{
    if (m_semid == -1)
    {
        return -1;
    }

    /* 加锁 */
    for(;;) {
        struct sembuf sops;
        sops.sem_num = pos;
        sops.sem_op = -1;
        sops.sem_flg = SEM_UNDO;

        int ret = semop(m_semid, &sops, cnt);
        if(ret<0) {
			if(errno == EINTR) {
				continue;
			}
			else {
                return -1;
			}
        } else {
            break;
        }
    }

    return CLockEx::Lock();
}

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
int CSemLockEx::UnLock(int pos, int cnt)
{
    if (m_semid == -1)
    {
        return -1;
    }

    /* 释放锁 */
    for(;;) {
        struct sembuf sops;
        sops.sem_num = pos;
        sops.sem_op = 1;
        sops.sem_flg = SEM_UNDO;

        int ret = semop(m_semid, &sops, cnt);
        if(ret<0) {
			if(errno == EINTR) {
				continue;
			}
			else {
                return -1;
			}
        } else {
            break;
        }
    }

    return CLockEx::UnLock();
}

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
int CSemLockEx::GetValue(unsigned short * value, int cnt)
{
	int ret = semctl(m_semid, cnt, GETALL, value);
	if(ret < 0) {
		return -1;
	}

	return 0;
}

CFileLock::CFileLock(const char * fname)
{
    m_fd = open(fname, O_RDWR | O_CREAT, 0666);
}

CFileLock::CFileLock(int fd) : m_fd(fd)
{
}

CFileLock::~CFileLock()
{
}

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
int CFileLock::Lock()
{
    if (m_fd == -1)
    {
        return -1;
    }

    /* 加锁 */
    int ret = flock(m_fd, LOCK_EX);
    if (ret < 0)
    {
        return -1;
    }

    return CLock::Lock();
}

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
int CFileLock::UnLock()
{
    if (m_fd == -1)
    {
        return -1;
    }

    /* 加锁 */
    int ret = flock(m_fd, LOCK_UN);
    if (ret < 0)
    {
        return -1;
    }

    return CLock::UnLock();
}

CFileLockEx::CFileLockEx(const char * fname, short ltype) : m_type(ltype)
{
    m_fd = open(fname, O_RDWR | O_CREAT, 0666);
}

CFileLockEx::CFileLockEx(int fd, short ltype) : m_fd(fd), m_type(ltype)
{
}

CFileLockEx::~CFileLockEx()
{
}

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
int CFileLockEx::Lock(int pos, int cnt)
{
    if (m_fd == -1)
    {
        return -1;
    }

    /* 加锁 */
    struct flock stLock;
        
    stLock.l_type = m_type; /* F_RDLCK,F_WRLCK,F_UNLCK */
    stLock.l_whence = SEEK_SET; /* SEEK_SET,SEEK_CUR,SEEK_END */
    stLock.l_start = pos; /* byte offset,relative to whence */
    stLock.l_len = cnt; /* bytes,0 means to EOF */
    int ret = fcntl(m_fd, F_SETLK, &stLock);
    if (ret < 0)
    {
        return -1;
    }

    return CLockEx::Lock();
}

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
int CFileLockEx::UnLock(int pos, int cnt)
{
    if (m_fd == -1)
    {
        return -1;
    }

    /* 加锁 */
    struct flock stLock;
        
    stLock.l_type = m_type; /* F_RDLCK,F_WRLCK,F_UNLCK */
    stLock.l_whence = SEEK_SET; /* SEEK_SET,SEEK_CUR,SEEK_END */
    stLock.l_start = pos; /* byte offset,relative to whence */
    stLock.l_len = cnt; /* bytes,0 means to EOF */
    int ret = fcntl(m_fd, F_UNLCK, &stLock);
    if (ret < 0)
    {
        return -1;
    }

    return CLockEx::UnLock();
}

CIDLockEx::CIDLockEx(int num)
{
    m_num = num;

    /* 初始化互斥锁 */
    pthread_mutex_init(&m_mutex, NULL);

    m_lockid = (int *)malloc(m_num * sizeof(int));
    memset(m_lockid, 0, m_num * sizeof(int));

    m_waitnum = (int *)malloc(m_num * sizeof(int));
    memset(m_waitnum, 0, m_num * sizeof(int));

    m_lockflag = (int *)malloc(m_num * sizeof(int));
    memset(m_lockflag, 0, m_num * sizeof(int));

    m_wqueue = (pthread_cond_t *)malloc(m_num * sizeof(pthread_cond_t));

    for (int i=0; i<m_num; i++)
    {
        pthread_cond_init(&(m_wqueue[i]), NULL);
    }
}

CIDLockEx::~CIDLockEx()
{
    if (m_wqueue)
    {
        for (int i=0; i<m_num; i++)
        {
            pthread_cond_destroy(&(m_wqueue[i]));
        }

        free(m_wqueue);
        m_wqueue = NULL;
    }

    if (m_lockflag)
    {
        free(m_lockflag);
        m_lockflag = NULL;
    }

    if (m_waitnum)
    {
        free(m_waitnum);
        m_waitnum = NULL;
    }

    if (m_lockid)
    {
        free(m_lockid);
        m_lockid = NULL;
    }

    /* 释放互斥锁 */
    pthread_mutex_destroy(&m_mutex);
}

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
int CIDLockEx::Lock(int lockId, int cnt)
{
    int i;

    pthread_mutex_lock(&m_mutex);
    for(i=0; i<m_num; i++){
        if( lockId == m_lockid[i]){
            m_waitnum[i]++;

            if(m_lockflag[i]==0){
                m_lockflag[i]=1;
                pthread_mutex_unlock(&m_mutex);
                return 0;
            }

            while(1){
                pthread_cond_wait( &m_wqueue[i], &m_mutex);
                if (m_lockflag[i]==0){
                    m_lockflag[i]=1;
                    pthread_mutex_unlock(&m_mutex);
                    return 0;
                }
            }
        }
    }

    for(i=0; i<m_num; i++){
        /* 虏陽抴aitnum[i] 为 0 */
        if( 0 == m_lockid[i]){
            m_lockid[i]=lockId; 
            m_waitnum[i]=1;
            m_lockflag[i]=1;
            pthread_mutex_unlock(&m_mutex);
            return 0;
        }
    }

    return CLockEx::Lock(lockId, 1);
}

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
int CIDLockEx::UnLock(int lockId, int cnt)
{
    int i;

    pthread_mutex_lock(&m_mutex);
    for(i=0; i<m_num; i++){
        if( lockId == m_lockid[i]){
            m_waitnum[i]--;
            m_lockflag[i]=0;
            if(m_waitnum[i] > 0)
                pthread_cond_signal(&m_wqueue[i]);
            else
                m_lockid[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&m_mutex);

    return CLockEx::UnLock(lockId, 1);
}
#endif
