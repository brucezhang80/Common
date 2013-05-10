#ifndef _TIMER_THREAD_H_
#define _TIMER_THREAD_H_

#include "lock.h"
#include "runable.h"
#include "sp_util.h"


struct _hash_obj;

struct _hash_key
{
	// obj.key或timer.key
	unsigned long _key;

	// 双向链表
	// 不论是超时或正常处理节点时，都可以快速更新两个hash
	_hash_obj *_prev;
	_hash_obj *_next;
};

// hash节点信息
struct _hash_obj
{
	// 数据包指针
	void *obj;

	// 在空闲队列时，指向下一个空闲节点
	// 没在空闲队列时，则下一个节点为NULL
	_hash_obj   *_next_obj;

	// 一个节点可以通过时间或key来查找
	_hash_key	_key_timer;		// 时间为key的hash
	_hash_key	_key_obj;		// obj.key为key的hash
};

// 空闲队列,  链表由hash中的_next_obj组成
struct _list_obj
{
	_hash_obj *_head;	// 队列头节点指针
	_hash_obj *_tail;	// 队列尾节点指针
};

class CTimerThread : public CRunable
{
private:
	bool running;
	
private:
	// 线程锁
	// 这里没有使用CAutoLock, 因为CAutoLock的性能比CThreadLock下降30%左右
	// TimerThread主要消耗在bucket的定位上，CAutoLock的性能损失也可以考虑不计
	CThreadLock m_lock;

	// m_ppTimerHash以相对毫秒数为bucket值
	// m_ppTimerHash[0]为标准时间每小时的0,10,20,30,40,50分0.000秒
	// 而每一个bucket表示在这10秒内的相对毫秒
	// 如m_ppTimerHash[10000]表示在这10分钟内的第10000毫秒
	_hash_obj **m_ppTimerHash;

	// m_ppObjHash以key做hash散列
	_hash_obj **m_ppObjHash;

	// 空闲hash节点队列
	_list_obj m_IdleList;

	unsigned int m_nHashSize;	// hash大小
	unsigned int m_nBucket;		// hash桶数
	unsigned int m_nTimeBucket;	// 时间hash桶数

	TUINT64 m_nLastMSec;	// 上次nCheckTimerHash的毫秒数

private:
	void Run();

	int nGetMsec();
	int nToMsec(TUINT64 nAbsMsec);

	int nCheckTimerHash();

	int nDeleteAllObj();
	
public:

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
	***************************************************************************/
	CTimerThread(unsigned int size, unsigned int bucket, unsigned int timebucket=60*1000*10ul);


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
	 *--------------------------------------------------------------------------
	***************************************************************************/
	~CTimerThread();


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
	***************************************************************************/
	int nPut(unsigned long key, void *p, unsigned int msec=0 );


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
	***************************************************************************/
	void *pvGet(unsigned long key);

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
	***************************************************************************/
	virtual void vHandleTimeout(void *p);

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
	***************************************************************************/
	virtual int nHashFunc(unsigned long key);

};

#endif
