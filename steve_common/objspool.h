/***************************************************************************
 * Copyright    : Shenzhen Tencent Co.Ltd.
 *
 * Program ID   : objspool.h
 *
 * Description  : 对象池
 *				  适用于单进程多线程共用数据区(非共享内存)的应用模式
 *				  用于统一管理对象
 *
 * Version      : 	
 *
 * Functions	: 	virtual T* newobject()		obj生成函数，继承类自定义
 *   				int init() 					初始化obj池
 * 					int release(T* elem) 		释放obj，obj放回对象池
 * 					T* get()					从obj池中取一个空闲obj
 *					int get_idle_num() 			取obj池中空闲obj数
 * *
 * Note :	
 *
 * Modification Log:
 * DATE         AUTHOR          DESCRIPTION
 *--------------------------------------------------------------------------
 * 2008-06-09   luckylin		initial
***************************************************************************/
#ifndef __OBJSPOOL_H_
#define __OBJSPOOL_H_

#include "lock.h"
#include <math.h>


template <class T>
class CObjsPool
{
	class _buf_node
	{
		struct _node_ctx
		{
#ifdef _DOUBLY_LINKED_LIST
			_buf_node *_prev;
#endif			
			_buf_node *_next;
		};

		public:
			// 对象指针
			T *_elem;

			// list或hash节点的上下游节点指针
			_node_ctx	_list;
			_node_ctx	_hash;

		public:
			_buf_node()
			{
				_elem = NULL;

#ifdef _DOUBLY_LINKED_LIST
				_list._prev = NULL;
				_hash._prev = NULL;
#endif
				_list._next = NULL;
				_hash._next = NULL;
			}
			~_buf_node()
			{
				if ( NULL != _elem )
					delete _elem;
				_elem = NULL;

#ifdef _DOUBLY_LINKED_LIST
				_list._prev = NULL;
				_hash._prev = NULL;
#endif
				_list._next = NULL;
				_hash._next = NULL;
			};
	};

	struct _node_list
	{
		_buf_node *_head;	// 队列头节点指针
		_buf_node *_tail;	// 队列尾节点指针
	};
protected:
	CThreadLock m_lock; /* 线程锁 */

	int m_num; /* 对象数目 */
    int m_idle_num; /* 空闲对象数目 */

	// m_ppObjHash以elem指针做hash的key进行散列
	_buf_node **m_ppObjHash;	/* 用hash做忙队列，可以减少出队列时查找 */
	unsigned int m_nBucket;		// hash桶数

	// 空闲hash节点队列
	_node_list m_idle;	/* 闲队列 */

public:
	CObjsPool(int num) 
	{
        m_num = num; /* 对象数目 */
        m_idle_num = num; /* 空闲对象数目 */

		m_nBucket  = num;
		if ( 0 == m_nBucket )
		{
			// 自动计算bucket数
			// 按hash size的1/5作为bucket数
			m_nBucket = GetBiggestPrime(num/5);
			if ( 10 >= m_nBucket )
			{
				// 最小bucket数为7
				m_nBucket = 7;
				
				// 如果不用上面的质数做bucket
				// 可以在此自定义
				// m_nBucket = xxx;        
			}
		}
		
		// 如果不用上面的质数做bucket
		// 可以在此自定义
		// m_nBucket = xxx;
        //printf("bucket=%d\n", m_nBucket);

        m_idle._head = NULL;
        m_idle._tail = NULL;        
    };

	virtual ~CObjsPool() 
	{
        m_lock.Lock();

		_buf_node *node = NULL;

        /* 清除忙队列(hash) */
        for (int i=0; i<m_num; i++)
	    {
	    	// 遍历每个桶
    	    node = m_ppObjHash[i%m_nBucket];
    	    while( NULL != node )
    	    {
    	    	// 保存下一个节点
            	_buf_node *t = node->_hash._next;

    	    	delete node;	//同时删除对象

				node = t;
    	    }
        }

        /* 清除空闲队列(list) */
        node = m_idle._head;
        while( NULL != node )
        {
   	    	// 保存下一个节点
            _buf_node *t = node->_list._next;

            delete node;	//同时删除对象

            node = t;
        }

		// 清除hash bucket
        delete [] m_ppObjHash;

        m_lock.UnLock();
    };


public:
    virtual T* newobject() = 0;

public:
	/***************************************************************************
	 * Function name: init
	 *
	 * Description: 初始化对象池
	 *				为node池，object, hash分配内存
	 *
	 * Parameters:  
	 *
	 * Return Value: 
	 *			0	成功
	 *			-1	没有空间,初始化_buf_node失败
	 *			-2	没有空间,初始化object失败
	 *			-3	没有空间,初始化hash bucket失败
	 *
	 * Note: 对象池大小在初始化CObjsPool中设定
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	int init() {
        /* 创建对象池 */
        for( int i=0; i<m_num; i++)
        {
        	// 创建节点
            _buf_node *node = new _buf_node;
			if ( NULL == node )
			{
				//printf("CObjsPool::init(), new _buf_node, i=%d failure\n", i);
				return -1;
			}
			
        	// 创建对象, newobject在继承类中定义
            node->_elem = newobject();
            if (NULL == node)
            {
				//printf("CObjsPool::init(), newobject(), i=%d failure\n", i);
				return -2;
			}

			/* 放入空闲队列 */
			AddToIdleQueue(node);
		}

		// 创建hash bucket
		m_ppObjHash = new _buf_node *[m_nBucket];
		if ( NULL == m_ppObjHash )
		{
			//printf("CObjsPool::init(), new m_ppObjHash bucket=%d failure\n", m_nBucket);
			return -3;
		}
		// 初始化每个bucket
		for( unsigned int i=0; i<m_nBucket; i++)
			m_ppObjHash[i] = NULL;

		return 0;
	}

	/***************************************************************************
	 * Function name: release
	 *
	 * Description: 释放对象
	 *				将对象重新放入对象池中
	 *
	 * Parameters:  
	 *			T* elem		对象指针
	 *
	 * Return Value: 
	 *			0	成功，
	 *			-1	失败，elem参数无效
	 *			-2	失败，没有发现该elem
	 *
	 * Note: elem指针必须是由CObjsPool创建的，如果只是数据相同，但指针不是CObjsPool创建的，则release会失败
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	int release(T* elem) 
	{
		int ret = 0;
		
		if (elem == NULL)
		{
			return -1;
		}

		m_lock.Lock();

		/* 从忙队列找对象 */
		_buf_node *node = RemoveFromBusyQueue(elem);

		/* 放入空闲队列 */
		if ( NULL != node )
		{
			AddToIdleQueue(node);

			m_idle_num++;
		}
		else
		{
			ret = -2;
		}

		m_lock.UnLock();

		return ret;
	};
	
	/***************************************************************************
	 * Function name: get
	 *
	 * Description: 从对象池中取一个空闲对象
	 *
	 * Parameters:  
	 *
	 * Return Value: 
	 *			T* elem		对象指针
	 *				not NULL 	成功，
	 *				NULL		失败，没有空闲对象
	 *
	 * Note: 
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	T* get() {
		_buf_node *node = NULL;

		T *elem = NULL;
		m_lock.Lock();		

		/* 闲队列移出头对象 */
		node = m_idle._head;
		if (NULL != node)
		{
			/* 放入忙队列 */
			AddToBusyQueue(node);

			m_idle_num--;

			// 调整队列首节点
			m_idle._head = node->_list._next;
			
			// 如果首节点为空，则尾节点也一定为空
			if ( NULL == node->_list._next )
				m_idle._tail = NULL;

			// 取得空闲对象
			elem = node->_elem;
		}

		m_lock.UnLock();

		return elem;
	};

	/***************************************************************************
	 * Function name: get_num
	 *
	 * Description: 取对象池中的对象总数
	 *
	 * Parameters:  
	 *
	 * Return Value: 
	 *			int		对象总数，与初始化时的数量相同
	 *
	 * Note: 
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	int get_num() { return m_num; };

	/***************************************************************************
	 * Function name: get_idle_num
	 *
	 * Description: 取对象池中的空闲对象数目
	 *
	 * Parameters:  
	 *
	 * Return Value: 
	 *			int		空闲对象数目
	 *
	 * Note: 在多线程状态下，此值不保证实时准确
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	int get_idle_num() { return m_idle_num; };

	/***************************************************************************
	 * Function name: get_used_num
	 *
	 * Description: 取对象池中的已使用对象数目
	 *
	 * Parameters:	
	 *
	 * Return Value: 
	 *			int 	已使用对象数目
	 *
	 * Note: 在多线程状态下，此值不保证实时准确
	 * 
	 * Modification Log:
	 * DATE 		 AUTHOR 		  DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	int get_used_num() { return m_num-m_idle_num; };

private:
	/***************************************************************************
	 * Function name: AddToIdleQueue
	 *
	 * Description: 将节点(包括对象)放入空闲队列尾
	 *
	 * Parameters:  
	 *			_buf_node *node		空闲节点的指针
	 *
	 * Return Value: 
	 *			0	成功
	 *			-1	失败，node参数无效
	 *
	 * Note: 
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	int AddToIdleQueue(_buf_node *node)
	{
		if ( NULL == node )
			return -1;
			
		if ( NULL == m_idle._tail )
		{
			// 首节点
			m_idle._head = node;
			m_idle._tail = node;

#ifdef _DOUBLY_LINKED_LIST
			// 本节点的上一节点为空节点
			node->_list._prev = NULL;
#endif
			// 本节点的下一节点为空节点
			node->_list._next = NULL;
		}
		else
		{
			// 当前尾节点的下一节点为本节点
			m_idle._tail->_list._next = node;

#ifdef _DOUBLY_LINKED_LIST
			// 本节点的上一节点为当前尾节点
			node->_list._prev = m_idle._tail;
#endif
			// 本节点的下一节点为空节点
			node->_list._next = NULL;
			
			// 设本节点为尾节点
			m_idle._tail = node;
		}

		return 0;
	};

	/***************************************************************************
	 * Function name: AddToBusyQueue
	 *
	 * Description: 将节点(包括对象)放入忙hash中
	 *
	 * Parameters:  
	 *			_buf_node *node		已被使用的节点的指针
	 *
	 * Return Value: 
	 *			0	成功
	 *			-1	失败，node参数无效
	 *
	 * Note: 
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	int AddToBusyQueue(_buf_node *node)
	{
		if ( NULL == node )
			return -1;

		// 计算hash bucket
		unsigned int h = (((size_t)node->_elem)/sizeof(T*))%m_nBucket;
		_buf_node *t  = m_ppObjHash[h];
		if ( NULL == t )
		{
			// bucket链表首指针
			m_ppObjHash[h] = node;
			node->_hash._next = NULL;
#ifdef _DOUBLY_LINKED_LIST
			node->_hash._prev = NULL;
#endif
		}
		else
		{
			// 遍历到bucket链表尾指针
			while( NULL != t->_hash._next )
				t = t->_hash._next;

			// 将该节点放到链表尾
			// 当前尾节点的下一个节点是本节点(本节点成为尾节点)
			t->_hash._next = node;
			
#ifdef _DOUBLY_LINKED_LIST
			// 本节点的上一个节点为当前尾节点
			node->_hash._prev = t;
#endif
		}

		return 0;
	}

	/***************************************************************************
	 * Function name: RemoveFromBusyQueue
	 *
	 * Description: 将对象(包括节点)从忙hash中去掉
	 *
	 * Parameters:  
	 *			T *elem		对象指针
	 *
	 * Return Value: 
	 *			0	成功
	 *			-1	失败，elem参数无效
	 *
	 * Note: elem指针必须是由CObjsPool创建的，如果只是数据相同，但指针不是CObjsPool创建的，则RemoveFromBusyQueue会失败
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	_buf_node *RemoveFromBusyQueue(T *elem)
	{
		if ( NULL == elem )
			return NULL;

		// 计算hash bucket
		unsigned int h = ((size_t)elem/sizeof(T*))%m_nBucket;
		_buf_node *t  = m_ppObjHash[h];
		
#ifndef _DOUBLY_LINKED_LIST
		_buf_node *prev = t;
#endif
		while( NULL != t )
		{			
			// 找此节点
			if ( elem == t->_elem )
			{
				// 将此节点从bucket链表中去掉				
				// 本节点是首节点
				if ( t == m_ppObjHash[h] )
				{
					// 将下一个节点设为首节点
					m_ppObjHash[h] = t->_hash._next;
					
#ifdef _DOUBLY_LINKED_LIST
					// 首节点的上一个节点为空
					if ( NULL != t->_hash._next )
						t->_hash._next->_hash._prev = NULL;
#endif
				}	
				else
				{
					// 将本节点的下游节点设为上一个节点的下游节点
#ifdef _DOUBLY_LINKED_LIST					
					if ( NULL != t->_hash._prev )
						t->_hash._prev->_hash._next = t->_hash._next;

					// 将本节点的上游节点设为下一个节点的上游节点	
					if ( NULL != t->_hash._next )
						t->_hash._next->_hash._prev = t->_hash._prev;
#else
					if ( NULL != prev )
						prev->_hash._next = t->_hash._next;
#endif					
				}
			
				// 将节点放入空闲队列
#ifdef _DOUBLY_LINKED_LIST
				t->_hash._prev = NULL;
#endif
				t->_hash._next = NULL;

				break;
			}
			else
			{
#ifndef _DOUBLY_LINKED_LIST
				// 在处理下一个节点前，先保存本节点为上一个节点
				prev = t;
#endif
				// 遍历此bucket
				t = t->_hash._next;
			}
		}

		return t;
	}

	/***************************************************************************
	 * Function name: GetBiggestPrime
	 *
	 * Description: 获取离maxNum最接近的最大质数
	 *
	 * Parameters:  
	 *		unsigned maxNum		最大数值
	 *
	 * Return Value:  
	 *		int 	获取离maxNum最接近的最大质数
	 *
	 * Note: 
	 *
	***************************************************************************/
	unsigned GetBiggestPrime(unsigned maxNum)
	{	
		//获取离maxNum最接近的最大质数
		//检查参数
		// ...
			
		//测试离指定数最近的质数
		unsigned i = 0;
		for(i=maxNum;i>=2;i--)
		{
			unsigned end=(unsigned)(sqrt(i)+1);
			unsigned j=0;
			for(j=2;j<end;j++)
			{
				if(0 == i%j)
				{
					break;
				}
			}
			if(j==end)//find the max prime
			{			
				return i;
			}
		}
		
		return 0;
	}

};

#endif

