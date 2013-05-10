/***************************************************************************
 * Copyright    : Shenzhen Tencent Co.Ltd.
 *
 * Program ID   : objspool.h
 *
 * Description  : �����
 *				  �����ڵ����̶��̹߳���������(�ǹ����ڴ�)��Ӧ��ģʽ
 *				  ����ͳһ�������
 *
 * Version      : 	
 *
 * Functions	: 	virtual T* newobject()		obj���ɺ������̳����Զ���
 *   				int init() 					��ʼ��obj��
 * 					int release(T* elem) 		�ͷ�obj��obj�Żض����
 * 					T* get()					��obj����ȡһ������obj
 *					int get_idle_num() 			ȡobj���п���obj��
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
			// ����ָ��
			T *_elem;

			// list��hash�ڵ�������νڵ�ָ��
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
		_buf_node *_head;	// ����ͷ�ڵ�ָ��
		_buf_node *_tail;	// ����β�ڵ�ָ��
	};
protected:
	CThreadLock m_lock; /* �߳��� */

	int m_num; /* ������Ŀ */
    int m_idle_num; /* ���ж�����Ŀ */

	// m_ppObjHash��elemָ����hash��key����ɢ��
	_buf_node **m_ppObjHash;	/* ��hash��æ���У����Լ��ٳ�����ʱ���� */
	unsigned int m_nBucket;		// hashͰ��

	// ����hash�ڵ����
	_node_list m_idle;	/* �ж��� */

public:
	CObjsPool(int num) 
	{
        m_num = num; /* ������Ŀ */
        m_idle_num = num; /* ���ж�����Ŀ */

		m_nBucket  = num;
		if ( 0 == m_nBucket )
		{
			// �Զ�����bucket��
			// ��hash size��1/5��Ϊbucket��
			m_nBucket = GetBiggestPrime(num/5);
			if ( 10 >= m_nBucket )
			{
				// ��Сbucket��Ϊ7
				m_nBucket = 7;
				
				// ������������������bucket
				// �����ڴ��Զ���
				// m_nBucket = xxx;        
			}
		}
		
		// ������������������bucket
		// �����ڴ��Զ���
		// m_nBucket = xxx;
        //printf("bucket=%d\n", m_nBucket);

        m_idle._head = NULL;
        m_idle._tail = NULL;        
    };

	virtual ~CObjsPool() 
	{
        m_lock.Lock();

		_buf_node *node = NULL;

        /* ���æ����(hash) */
        for (int i=0; i<m_num; i++)
	    {
	    	// ����ÿ��Ͱ
    	    node = m_ppObjHash[i%m_nBucket];
    	    while( NULL != node )
    	    {
    	    	// ������һ���ڵ�
            	_buf_node *t = node->_hash._next;

    	    	delete node;	//ͬʱɾ������

				node = t;
    	    }
        }

        /* ������ж���(list) */
        node = m_idle._head;
        while( NULL != node )
        {
   	    	// ������һ���ڵ�
            _buf_node *t = node->_list._next;

            delete node;	//ͬʱɾ������

            node = t;
        }

		// ���hash bucket
        delete [] m_ppObjHash;

        m_lock.UnLock();
    };


public:
    virtual T* newobject() = 0;

public:
	/***************************************************************************
	 * Function name: init
	 *
	 * Description: ��ʼ�������
	 *				Ϊnode�أ�object, hash�����ڴ�
	 *
	 * Parameters:  
	 *
	 * Return Value: 
	 *			0	�ɹ�
	 *			-1	û�пռ�,��ʼ��_buf_nodeʧ��
	 *			-2	û�пռ�,��ʼ��objectʧ��
	 *			-3	û�пռ�,��ʼ��hash bucketʧ��
	 *
	 * Note: ����ش�С�ڳ�ʼ��CObjsPool���趨
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	int init() {
        /* ��������� */
        for( int i=0; i<m_num; i++)
        {
        	// �����ڵ�
            _buf_node *node = new _buf_node;
			if ( NULL == node )
			{
				//printf("CObjsPool::init(), new _buf_node, i=%d failure\n", i);
				return -1;
			}
			
        	// ��������, newobject�ڼ̳����ж���
            node->_elem = newobject();
            if (NULL == node)
            {
				//printf("CObjsPool::init(), newobject(), i=%d failure\n", i);
				return -2;
			}

			/* ������ж��� */
			AddToIdleQueue(node);
		}

		// ����hash bucket
		m_ppObjHash = new _buf_node *[m_nBucket];
		if ( NULL == m_ppObjHash )
		{
			//printf("CObjsPool::init(), new m_ppObjHash bucket=%d failure\n", m_nBucket);
			return -3;
		}
		// ��ʼ��ÿ��bucket
		for( unsigned int i=0; i<m_nBucket; i++)
			m_ppObjHash[i] = NULL;

		return 0;
	}

	/***************************************************************************
	 * Function name: release
	 *
	 * Description: �ͷŶ���
	 *				���������·���������
	 *
	 * Parameters:  
	 *			T* elem		����ָ��
	 *
	 * Return Value: 
	 *			0	�ɹ���
	 *			-1	ʧ�ܣ�elem������Ч
	 *			-2	ʧ�ܣ�û�з��ָ�elem
	 *
	 * Note: elemָ���������CObjsPool�����ģ����ֻ��������ͬ����ָ�벻��CObjsPool�����ģ���release��ʧ��
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

		/* ��æ�����Ҷ��� */
		_buf_node *node = RemoveFromBusyQueue(elem);

		/* ������ж��� */
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
	 * Description: �Ӷ������ȡһ�����ж���
	 *
	 * Parameters:  
	 *
	 * Return Value: 
	 *			T* elem		����ָ��
	 *				not NULL 	�ɹ���
	 *				NULL		ʧ�ܣ�û�п��ж���
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

		/* �ж����Ƴ�ͷ���� */
		node = m_idle._head;
		if (NULL != node)
		{
			/* ����æ���� */
			AddToBusyQueue(node);

			m_idle_num--;

			// ���������׽ڵ�
			m_idle._head = node->_list._next;
			
			// ����׽ڵ�Ϊ�գ���β�ڵ�Ҳһ��Ϊ��
			if ( NULL == node->_list._next )
				m_idle._tail = NULL;

			// ȡ�ÿ��ж���
			elem = node->_elem;
		}

		m_lock.UnLock();

		return elem;
	};

	/***************************************************************************
	 * Function name: get_num
	 *
	 * Description: ȡ������еĶ�������
	 *
	 * Parameters:  
	 *
	 * Return Value: 
	 *			int		�������������ʼ��ʱ��������ͬ
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
	 * Description: ȡ������еĿ��ж�����Ŀ
	 *
	 * Parameters:  
	 *
	 * Return Value: 
	 *			int		���ж�����Ŀ
	 *
	 * Note: �ڶ��߳�״̬�£���ֵ����֤ʵʱ׼ȷ
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	int get_idle_num() { return m_idle_num; };

	/***************************************************************************
	 * Function name: get_used_num
	 *
	 * Description: ȡ������е���ʹ�ö�����Ŀ
	 *
	 * Parameters:	
	 *
	 * Return Value: 
	 *			int 	��ʹ�ö�����Ŀ
	 *
	 * Note: �ڶ��߳�״̬�£���ֵ����֤ʵʱ׼ȷ
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
	 * Description: ���ڵ�(��������)������ж���β
	 *
	 * Parameters:  
	 *			_buf_node *node		���нڵ��ָ��
	 *
	 * Return Value: 
	 *			0	�ɹ�
	 *			-1	ʧ�ܣ�node������Ч
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
			// �׽ڵ�
			m_idle._head = node;
			m_idle._tail = node;

#ifdef _DOUBLY_LINKED_LIST
			// ���ڵ����һ�ڵ�Ϊ�սڵ�
			node->_list._prev = NULL;
#endif
			// ���ڵ����һ�ڵ�Ϊ�սڵ�
			node->_list._next = NULL;
		}
		else
		{
			// ��ǰβ�ڵ����һ�ڵ�Ϊ���ڵ�
			m_idle._tail->_list._next = node;

#ifdef _DOUBLY_LINKED_LIST
			// ���ڵ����һ�ڵ�Ϊ��ǰβ�ڵ�
			node->_list._prev = m_idle._tail;
#endif
			// ���ڵ����һ�ڵ�Ϊ�սڵ�
			node->_list._next = NULL;
			
			// �豾�ڵ�Ϊβ�ڵ�
			m_idle._tail = node;
		}

		return 0;
	};

	/***************************************************************************
	 * Function name: AddToBusyQueue
	 *
	 * Description: ���ڵ�(��������)����æhash��
	 *
	 * Parameters:  
	 *			_buf_node *node		�ѱ�ʹ�õĽڵ��ָ��
	 *
	 * Return Value: 
	 *			0	�ɹ�
	 *			-1	ʧ�ܣ�node������Ч
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

		// ����hash bucket
		unsigned int h = (((size_t)node->_elem)/sizeof(T*))%m_nBucket;
		_buf_node *t  = m_ppObjHash[h];
		if ( NULL == t )
		{
			// bucket������ָ��
			m_ppObjHash[h] = node;
			node->_hash._next = NULL;
#ifdef _DOUBLY_LINKED_LIST
			node->_hash._prev = NULL;
#endif
		}
		else
		{
			// ������bucket����βָ��
			while( NULL != t->_hash._next )
				t = t->_hash._next;

			// ���ýڵ�ŵ�����β
			// ��ǰβ�ڵ����һ���ڵ��Ǳ��ڵ�(���ڵ��Ϊβ�ڵ�)
			t->_hash._next = node;
			
#ifdef _DOUBLY_LINKED_LIST
			// ���ڵ����һ���ڵ�Ϊ��ǰβ�ڵ�
			node->_hash._prev = t;
#endif
		}

		return 0;
	}

	/***************************************************************************
	 * Function name: RemoveFromBusyQueue
	 *
	 * Description: ������(�����ڵ�)��æhash��ȥ��
	 *
	 * Parameters:  
	 *			T *elem		����ָ��
	 *
	 * Return Value: 
	 *			0	�ɹ�
	 *			-1	ʧ�ܣ�elem������Ч
	 *
	 * Note: elemָ���������CObjsPool�����ģ����ֻ��������ͬ����ָ�벻��CObjsPool�����ģ���RemoveFromBusyQueue��ʧ��
	 * 
	 * Modification Log:
	 * DATE          AUTHOR           DESCRIPTION
	 *--------------------------------------------------------------------------
	***************************************************************************/
	_buf_node *RemoveFromBusyQueue(T *elem)
	{
		if ( NULL == elem )
			return NULL;

		// ����hash bucket
		unsigned int h = ((size_t)elem/sizeof(T*))%m_nBucket;
		_buf_node *t  = m_ppObjHash[h];
		
#ifndef _DOUBLY_LINKED_LIST
		_buf_node *prev = t;
#endif
		while( NULL != t )
		{			
			// �Ҵ˽ڵ�
			if ( elem == t->_elem )
			{
				// ���˽ڵ��bucket������ȥ��				
				// ���ڵ����׽ڵ�
				if ( t == m_ppObjHash[h] )
				{
					// ����һ���ڵ���Ϊ�׽ڵ�
					m_ppObjHash[h] = t->_hash._next;
					
#ifdef _DOUBLY_LINKED_LIST
					// �׽ڵ����һ���ڵ�Ϊ��
					if ( NULL != t->_hash._next )
						t->_hash._next->_hash._prev = NULL;
#endif
				}	
				else
				{
					// �����ڵ�����νڵ���Ϊ��һ���ڵ�����νڵ�
#ifdef _DOUBLY_LINKED_LIST					
					if ( NULL != t->_hash._prev )
						t->_hash._prev->_hash._next = t->_hash._next;

					// �����ڵ�����νڵ���Ϊ��һ���ڵ�����νڵ�	
					if ( NULL != t->_hash._next )
						t->_hash._next->_hash._prev = t->_hash._prev;
#else
					if ( NULL != prev )
						prev->_hash._next = t->_hash._next;
#endif					
				}
			
				// ���ڵ������ж���
#ifdef _DOUBLY_LINKED_LIST
				t->_hash._prev = NULL;
#endif
				t->_hash._next = NULL;

				break;
			}
			else
			{
#ifndef _DOUBLY_LINKED_LIST
				// �ڴ�����һ���ڵ�ǰ���ȱ��汾�ڵ�Ϊ��һ���ڵ�
				prev = t;
#endif
				// ������bucket
				t = t->_hash._next;
			}
		}

		return t;
	}

	/***************************************************************************
	 * Function name: GetBiggestPrime
	 *
	 * Description: ��ȡ��maxNum��ӽ����������
	 *
	 * Parameters:  
	 *		unsigned maxNum		�����ֵ
	 *
	 * Return Value:  
	 *		int 	��ȡ��maxNum��ӽ����������
	 *
	 * Note: 
	 *
	***************************************************************************/
	unsigned GetBiggestPrime(unsigned maxNum)
	{	
		//��ȡ��maxNum��ӽ����������
		//������
		// ...
			
		//������ָ�������������
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

