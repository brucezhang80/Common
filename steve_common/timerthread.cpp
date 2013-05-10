/***************************************************************************
 * Copyright    : Shenzhen Tencent Co.Ltd.
 *
 * Program ID   : timerthread.cpp
 *
 * Description  : 定时器线程
 *				  适用于单进程多线程共用数据区(非共享内存)的应用模式
 *				  用于管理需要超时控制的数据包指针，
 *				  内部一个hash buf，两个hash(key hash, timer hash)，一个空闲链表
 *				  两个hash内采用双向链表，bucket中任一元素删除都只需两个链表操作，不需要遍历bucket中每个节点
 *
 *                如果编译预定义 _PTH_USLEEP_, 表示使用pth库中的pth_sleep, 需要libpth.a
 *				  pth_usleep 线程不安全，全局只能有一个线程使用, 闲时延时<0.5ms，忙时延时<1.5ms
 *
 * Version      : 
 *
 * Functions:   int nPut		加入需超时控制的数据包指针
 *				void *pvGet		取数据包指针
 *				vHandleTimeout	超时处理函数
 *
 * Note :	由于usleep或pth_usleep都是非高精度延时函数，所以<10ms级别的数据包超时时间精度都较低
 *			nPut的效率与bucket大小相关
 *			CPU的消耗主要是nPut的消耗，TimerThread本身使用<1%CPU
 *
 * Modification Log:
 * DATE         AUTHOR          DESCRIPTION
 *--------------------------------------------------------------------------
 
 * 2008-06-06   luckylin		initial
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "sp_util.h"
#include "timerthread.h"

#ifdef _PTH_USLEEP_
#include "pth.h"
#else
#include <unistd.h>
#endif

#ifndef int64
typedef __int64_t	int64;
#endif

#define _log_func	printf

int nUSleep(int usec)
{
#ifdef _PTH_USLEEP_
	/*
	* 注意:
	*      pth_usleep 线程不安全，全局只能有一个线程使用
	*      闲时延时<0.5ms，忙时延时<1.5ms
	*/
	static int init = 0;
	if ( 0 == init )
	{
		pth_init();
		init = 1;
	}
    return pth_usleep(usec);
#else
	/*
	*  usleep延时~4ms
	*/
    return usleep(usec);
#endif
}

// 缺省超时时间
#define MacDefaultTimeout 	4		// 毫秒

// 缺省最大超时时间
#define MacMaxTimeout	60*1000*10	// 毫秒


/***************************************************************************
 * Function name: CTimerThread
 *
 * Description: TimerThread构造函数
 *
 * Parameters:  
 * 		unsigned int size		hash 大小
 * 		unsigned int bucket		obj hash桶大小，如果hash size不是很大，建议bucket设一个接近或等于size的值
 *		unsigned int timebucket timer hash桶大小，缺省60*1000*10，即最多10分钟超时
 *
 * Return Value: 
 *
 * Note: 只支持long int 为key
 * 
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
CTimerThread::CTimerThread(unsigned int size, unsigned int bucket, unsigned int timebucket)
{	
	m_nHashSize = size;
	m_nBucket = bucket;
	m_nTimeBucket = timebucket;
	if ( 0 == m_nTimeBucket )
	{
		m_nTimeBucket = MacMaxTimeout;
	}

	m_nLastMSec = GetTime();

	// 空闲hash节点链表
	m_IdleList._head = NULL;
	m_IdleList._tail = NULL;

	// 初始化hash节点
	for( unsigned int i=0; i<size; ++i)
	{
		_hash_obj *obj = new _hash_obj;

		if ( NULL == obj )
		{
			_log_func("CTimerThread::CTimerThread(): alloc memory: _hash_obj * failure!\n");
			break;
		}

		memset(obj, 0, sizeof(_hash_obj));

		// 将hash节点挂入空闲链表
		if ( 0 == i )
		{
			// 第一个空闲节点
			m_IdleList._head = obj;
			m_IdleList._tail = obj;
		}
		if ( NULL != m_IdleList._tail )
		{
			m_IdleList._tail->_next_obj = obj;
			m_IdleList._tail = obj;
		}
	}
	
	// 初始化两个hash
	// 对象hash
	m_ppObjHash = new _hash_obj*[m_nBucket];
	if ( NULL == m_ppObjHash )
	{
		_log_func("CTimerThread::CTimerThread(): alloc memory: m_ppObjHash failure!\n");
	}
	for( unsigned int i=0; i<m_nBucket; ++i)
	{
		m_ppObjHash[i] = NULL;
	}

	// 时间hash
	m_ppTimerHash = new _hash_obj* [m_nTimeBucket];
	if ( NULL == m_ppTimerHash )
	{
		_log_func("CTimerThread::CTimerThread(): alloc memory: m_ppTimerHash failure!\n");
	}
	for( unsigned int i=0; i<m_nTimeBucket; ++i)
	{
		m_ppTimerHash[i] = NULL;
	}

	// 设循环标志
	running = true;
}

/***************************************************************************
 * Function name: CTimerThread
 *
 * Description: TimerThread释构函数
 *				控制主循环退出并释放hash节点
 *
 * Parameters:  
 *
 * Return Value: 
 *
 * Note: 
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
CTimerThread::~CTimerThread()
{
	// 设退出标志，并等待主循环退出
	running = false;
	usleep(10000);

	// 释放在hash中的obj指针，只需要释放ObjHash或TimerHash其中一个就可以了
	nDeleteAllObj();

	CAutoLock l(&m_lock);

	if ( NULL != m_ppTimerHash )
		delete [] m_ppTimerHash;
	m_ppTimerHash = NULL;

	if ( NULL != m_ppObjHash )
		delete [] m_ppObjHash;
	m_ppObjHash = NULL;

	_log_func("CTimerThread::~CTimerThread(): exit\n");
}

/***************************************************************************
 * Function name: nPut
 *
 * Description: 将数据包放入TimerHash中等待超时服务
 *
 * Parameters:  
 *		unsigned long key	数据包key
 *		void *p				数据包指针, obj hash与timer hash不保存数据包内容
 *		unsigned int msec	数据包的超时时间(毫秒), =0 表示使用default超时时间
 *
 * Return Value:  0		成功
 *				 -1		hash没有空间
 *
 * Note: 不支持相同key的情况
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
int CTimerThread::nPut(unsigned long key, void *p, unsigned int msec )
{
	// bucket
	int h;

	// 1.锁住资源
	m_lock.Lock();

	// 2. 申请一个未用的hash节点
	_hash_obj *hobj = m_IdleList._head;

	// 已经没有空闲hash节点
	if ( NULL == hobj )
	{
		m_lock.UnLock();
		return -1;
	}
	else
	{
		// 下一个空闲节点成为首节点
		m_IdleList._head = hobj->_next_obj;

		// 本节点已经出列，不再指向任何空闲节点
		hobj->_next_obj = NULL;

		// 如果首节点为空，则尾节点也一定为空
		if ( NULL == m_IdleList._head )
			m_IdleList._tail = NULL;
	}

	hobj->obj = p;

	unsigned int _msec = msec;
	if ( 0 == _msec )
	{
		_msec = MacDefaultTimeout;
	}

	// 计算在TimerHash中的bucket位置
	hobj->_key_timer._key = (nGetMsec()+_msec)%m_nTimeBucket;
	hobj->_key_timer._prev = NULL;
	hobj->_key_timer._next = NULL;

	hobj->_key_obj._key = key;
	hobj->_key_obj._prev = NULL;
	hobj->_key_obj._next = NULL;

	// 3.先放入ObjHash中
	// 计算在ObjHash中的bucket位置
	h =nHashFunc(key);

	// 首节点为空
	_hash_obj *obj = m_ppObjHash[h];
	if ( NULL == obj )
	{
		m_ppObjHash[h] = hobj;
	}
	else
	{
		// 首节点非空，则放到链尾
		while( NULL != obj->_key_obj._next )
		{
			obj = obj->_key_obj._next;
		}

		// 更新上下游节点指针
		obj->_key_obj._next = hobj;
		hobj->_key_obj._prev = obj;
	}

	// 4.再先放入TimerHash中
	h = hobj->_key_timer._key;

	// 首节点为空
	obj = m_ppTimerHash[h];
	if ( NULL == obj )
	{	
		m_ppTimerHash[h] = hobj;
	}
	else
	{
		// 首节点非空，放到链尾
		while( NULL != obj->_key_timer._next )
		{
			obj = obj->_key_timer._next;
		}

		// 更新上下游节点指针
		obj->_key_timer._next = hobj;
		hobj->_key_timer._prev = obj;
	}

	m_lock.UnLock();

	return 0;
}

/***************************************************************************
 * Function name: pvGet
 *
 * Description: 从hash中根据key取回数据包指针
 * 				成功后objhash、timerhash中的节点会被删除
 *
 * Parameters:  
 *		unsigned long key	数据包key
 *
 * Return Value:  
 *		void *	not null, 返回数据包指针
 *				null, 没有该key的数据包指针
 *
 * Note: 不支持相同key的情况
 * 		 当数据包正常返回时调用，如果不调用，则TimerThread中会对这个数据包进行超时，有可能做成业务逻辑混乱
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
void *CTimerThread::pvGet(unsigned long key)
{
	void *p = NULL;

	// 锁住资源
	m_lock.Lock();

	int h = nHashFunc(key);

	// bucket首节点
	_hash_obj *hobj  = m_ppObjHash[h];

	while( NULL != hobj )
	{
		// 将key值对应的节点
		if ( hobj->_key_obj._key == key )
		{
			// 更新ObjHash上下游节点指针
			_hash_obj *up_obj = hobj->_key_obj._prev;
			_hash_obj *down_obj = hobj->_key_obj._next;
			if ( (NULL==up_obj) && (NULL==down_obj) )
			{
				// 没有上下游节点，本节点为bucket首节点，且只有本节点
				m_ppObjHash[h] = NULL;
			}
			else
			if ( (NULL!=up_obj) && (NULL!=down_obj) )
			{
				// 有上下游节点,此节点为中间节点
				up_obj->_key_obj._next = down_obj;
				down_obj->_key_obj._prev = up_obj;
			}
			else
			if ( (NULL==up_obj) && (NULL!=down_obj) )
			{
				// 没有上游节点，但有下游节点，本节点为bucket首节点
				// 下游节点成为首节点
				down_obj->_key_obj._prev = NULL;
				m_ppObjHash[h] = down_obj;
			}
			else
			if ( (NULL!=up_obj) && (NULL==down_obj) )
			{
				// 有上游节点，但没有下游节点，本节点为bucket尾节点
				up_obj->_key_obj._next = NULL;
			}

			// 同时更新TimerHash上下游节点指针
			up_obj = hobj->_key_timer._prev;
			down_obj = hobj->_key_timer._next;
			if ( (NULL==up_obj) && (NULL==down_obj) )
			{
				// 没有上下游节点，本节点为bucket首节点，且只有本节点
				m_ppTimerHash[hobj->_key_timer._key] = NULL;
			}
			else
			if ( (NULL!=up_obj) && (NULL!=down_obj) )
			{
				// 有上下游节点,此节点为中间节点
				up_obj->_key_timer._next = down_obj;
				down_obj->_key_timer._prev = up_obj;
			}
			else
			if ( (NULL==up_obj) && (NULL!=down_obj) )
			{
				// 没有上游节点，但有下游节点，本节点为bucket首节点
				// 下游节点成为首节点
				down_obj->_key_timer._prev = NULL;
				m_ppTimerHash[hobj->_key_timer._key] = down_obj;
			}
			else
			if ( (NULL!=up_obj) && (NULL==down_obj) )
			{
				// 有上游节点，但没有下游节点，本节点为bucket尾节点
				up_obj->_key_timer._next = NULL;
			}

			// 返回obj对象
			p = hobj->obj;


			// 释放当前hash节点，重新挂放空闲队列
			// 如果尾节点非空，则将此hobj放入空闲链表中
			if ( NULL != m_IdleList._tail )
			{
				// 原尾节点的下一个空闲节点指向本节点
				m_IdleList._tail->_next_obj = hobj;
				// 本节点成为尾节点
				m_IdleList._tail = hobj;
			}
			else
			{
				// 尾节点为空，头节点也一定为空
				m_IdleList._head = hobj;
				m_IdleList._tail = hobj;
			}

			break;
		}
		else
		{
			// 下一个节点，直到没有
			hobj = hobj->_key_obj._next;
		}
	}

	m_lock.UnLock();

	return p;
}


/***************************************************************************
 * Function name: nGetMsec
 *
 * Description: 取当前时间的定时器相对毫秒数
 *
 * Parameters:  
 *
 * Return Value: 
 *		int		定时器相对毫秒数，即n分钟内的第m毫秒
 *
 * Note: 如果tv_usec>500，进行4舍5入调整，相对毫秒数+1
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------
 
***************************************************************************/
inline int CTimerThread::nGetMsec()
{
 	struct timeval now;
	gettimeofday(&now, 0);

	// usec 4舍5入调整，>500usec时，加1msec
	int m = 0;
	if ( (now.tv_usec%1000) > 500 )
		m = 1;
		
	// 当前时间%600(秒)即为10分钟内的秒数
	// (m_nTimeBucket/1000) = 最大超时秒数
	return ((now.tv_sec%(m_nTimeBucket/1000))*1000+now.tv_usec/1000+m);
}


/***************************************************************************
 * Function name: nToMsec
 *
 * Description: 将绝对毫秒数转为定时器相对毫秒数
 *
 * Parameters:  
 *		TUINT64 nAbsMsec	绝对毫秒数，即tv.tv_sec*1000+tv.tv_usec/1000
 *
 * Return Value: 
 *		int		定时器相对毫秒数，即n分钟内的第m毫秒
 *
 * Note: 
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------
 
***************************************************************************/
inline int CTimerThread::nToMsec(TUINT64 nAbsMsec)
{
	// (m_nTimeBucket/1000) = 最大超时秒数
	return (int)(((nAbsMsec/1000)%(m_nTimeBucket/1000))*1000+nAbsMsec%1000);
}


/***************************************************************************
 * Function name: nCheckTimerHash
 *
 * Description: 检查超时节点
 *				从上一次(毫秒数)bucket开始扫描TimerHash,直到本次(毫秒数)bucket为止，
 *              每个bucket表示某一毫秒(n秒内毫秒数)，在bucket中的节点表示在该毫秒数时，节点会超时
 *
 * Parameters:  
 *
 * Return Value:  
 *			0		成功
 *			-1		失败，ObjHash或TimerHash为空
 *
 * Note: 超时节点本身只含有业务数据包的指针，TimerThread抛出超时节点由虚函数vHandleTimeout处理
 *       超时的节点资源会被释放回收，在ObjHash及TimerHash中同时删除，放入空闲节点队列中被重新使用
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------
 
***************************************************************************/
int CTimerThread::nCheckTimerHash()
{
	if ( (NULL == m_ppTimerHash) || (NULL == m_ppObjHash) )
		return -1;
		
	// 取当前时间(毫秒)
	TUINT64 mSec = GetTime();

	// 与上次nCheckTimerHash的(毫秒)时间差
	int elapse = mSec - m_nLastMSec;

	// 锁住资源
	m_lock.Lock();

	// 由于nUSleep精度不高，所以需要从上次m_nLastMSec开始检查TimerHash，避免忙时有部分bucket未处理
	for(int i=1; i<=elapse; ++i)
	{
		// 计算需要检查的bucket
		// 在此bucket内的所有节点都已经超时
		int h = (nToMsec(m_nLastMSec)+i)%m_nTimeBucket;

		// bucket首节点
		_hash_obj *hobj  = m_ppTimerHash[h];
		while( NULL != hobj )
		{
			// 将超时节点抛出处理
			vHandleTimeout(hobj->obj);

			// 同时更新ObjHash上下游节点指针
			_hash_obj *up_obj = hobj->_key_obj._prev;
			_hash_obj *down_obj = hobj->_key_obj._next;
			int h1 = nHashFunc(hobj->_key_obj._key);
			if ( (NULL==up_obj) && (NULL==down_obj) )
			{
				// 没有上下游节点，本节点为bucket首节点，且只有本节点
				m_ppObjHash[h1] = NULL;
			}
			else
			if ( (NULL!=up_obj) && (NULL!=down_obj) )
			{
				// 有上下游节点,此节点为中间节点
				up_obj->_key_obj._next = down_obj;
				down_obj->_key_obj._prev = up_obj;
			}
			else
			if ( (NULL==up_obj) && (NULL!=down_obj) )
			{
				// 没有上游节点，但有下游节点，本节点为bucket首节点
				// 下游节点成为首节点
				down_obj->_key_obj._prev = NULL;
				m_ppObjHash[h1] = down_obj;
			}
			else
			if ( (NULL!=up_obj) && (NULL==down_obj) )
			{
				// 有上游节点，但没有下游节点，本节点为bucket尾节点
				up_obj->_key_obj._next = NULL;
			}


			// 释放当前hash节点，重新挂入空闲队列
			// 如果尾节点非空，则将此hobj放入空闲链表中
			if ( NULL != m_IdleList._tail )
			{
				// 原尾节点的下一个空闲节点指向本节点
				m_IdleList._tail->_next_obj = hobj;
				// 本节点成为尾节点
				m_IdleList._tail = hobj;
			}
			else
			{
				// 尾节点为空，头节点也一定为空
				m_IdleList._head = hobj;
				m_IdleList._tail = hobj;
			}

			// 本bucket的下一个节点，直到没有
			hobj = hobj->_key_timer._next;
		}

		// 本bucket节点全部被处理，首节点为空
		m_ppTimerHash[h] = NULL;
	}

	// 更新最后一次处理的毫秒数
	m_nLastMSec = mSec;

	m_lock.UnLock();

	return 0;
}


/***************************************************************************
 * Function name: nDeleteAllObj 
 *
 * Description: 删除所有Hash节点
 *              Hash节点分布在ObjHash/TimerHash及IdleList中
 *		 		由于ObjHash或TimerHash是共用hash节点，所以只需要遍历删除ObjHash或TimerHash中的其中一个
 *
 * Parameters:  
 *
 * Return Value:  
 *			>0		成功删除节点的个数，应该=hash size
 *
 * Note: 本操作只在TimerThread退出时使用
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
int CTimerThread::nDeleteAllObj()
{
	int hash_count = 0;
	int idle_count = 0;

	m_lock.Lock();

	// 遍历所有bucket
	for( unsigned int i=0; i< m_nBucket; ++i)
	{
		_hash_obj *hobj = m_ppObjHash[i];

		while ( NULL != hobj )
		{		
			_hash_obj *tobj = hobj->_key_obj._next;

			delete hobj;
			
			//删除的节点计数
			++hash_count;
			
			hobj = tobj;
		}
	}

	// 遍历空闲链表
	_hash_obj *hobj = m_IdleList._head;
	while( NULL != hobj )
	{
		_hash_obj *tobj = hobj->_next_obj;

		delete hobj;
		//删除的节点计数
		++idle_count;
		
		hobj = tobj;
	}
	
	m_lock.UnLock();

	_log_func("CTimerThread::nDeleteAllObj(): delete hash=%d, ilde=%d \n", hash_count, idle_count);

	return (hash_count+idle_count);
}


/***************************************************************************
 * Function name: vHandleTimeout
 *
 * Description: 超时处理函数
 *
 * Parameters:  
 *		void *p			已经超时的数据包
 *
 * Return Value:  
 *
 * Note: !!! 必须重载vHandleTimeout !!!
 *		 处理函数的逻辑应该尽量轻，避免逻辑过重导致TimerThread无法处理其他的超时节点
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
void CTimerThread::vHandleTimeout(void *p)
{
	// do nothing
}


/***************************************************************************
 * Function name: nHashFunc
 *
 * Description: hash函数
 *				计算key在hash中的bucket值
 *
 * Parameters:  
 *		unsigned long key	数据包key
 *
 * Return Value:  
 *		int 	散列到hash中的bucket值
 *
 * Note: 可以重载此函数
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
int CTimerThread::nHashFunc(unsigned long key)
{
	return (int)(key%m_nBucket);
}


/***************************************************************************
 * Function name: Run
 *
 * Description: 线程主循环处理函数
 *
 * Parameters:  
 *
 * Return Value:  
 *
 * Note: TimerThread退出时设置running控制主循环退出
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
void CTimerThread::Run()
{
	while(true == running)
	{
		nUSleep(1);

		/* 检查TimerHash是否有超时 */
		nCheckTimerHash();
	}

	_log_func("CTimerThread::Run(): exit\n");
}

