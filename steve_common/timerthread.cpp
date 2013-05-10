/***************************************************************************
 * Copyright    : Shenzhen Tencent Co.Ltd.
 *
 * Program ID   : timerthread.cpp
 *
 * Description  : ��ʱ���߳�
 *				  �����ڵ����̶��̹߳���������(�ǹ����ڴ�)��Ӧ��ģʽ
 *				  ���ڹ�����Ҫ��ʱ���Ƶ����ݰ�ָ�룬
 *				  �ڲ�һ��hash buf������hash(key hash, timer hash)��һ����������
 *				  ����hash�ڲ���˫������bucket����һԪ��ɾ����ֻ�������������������Ҫ����bucket��ÿ���ڵ�
 *
 *                �������Ԥ���� _PTH_USLEEP_, ��ʾʹ��pth���е�pth_sleep, ��Ҫlibpth.a
 *				  pth_usleep �̲߳���ȫ��ȫ��ֻ����һ���߳�ʹ��, ��ʱ��ʱ<0.5ms��æʱ��ʱ<1.5ms
 *
 * Version      : 
 *
 * Functions:   int nPut		�����賬ʱ���Ƶ����ݰ�ָ��
 *				void *pvGet		ȡ���ݰ�ָ��
 *				vHandleTimeout	��ʱ������
 *
 * Note :	����usleep��pth_usleep���ǷǸ߾�����ʱ����������<10ms��������ݰ���ʱʱ�侫�ȶ��ϵ�
 *			nPut��Ч����bucket��С���
 *			CPU��������Ҫ��nPut�����ģ�TimerThread����ʹ��<1%CPU
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
	* ע��:
	*      pth_usleep �̲߳���ȫ��ȫ��ֻ����һ���߳�ʹ��
	*      ��ʱ��ʱ<0.5ms��æʱ��ʱ<1.5ms
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
	*  usleep��ʱ~4ms
	*/
    return usleep(usec);
#endif
}

// ȱʡ��ʱʱ��
#define MacDefaultTimeout 	4		// ����

// ȱʡ���ʱʱ��
#define MacMaxTimeout	60*1000*10	// ����


/***************************************************************************
 * Function name: CTimerThread
 *
 * Description: TimerThread���캯��
 *
 * Parameters:  
 * 		unsigned int size		hash ��С
 * 		unsigned int bucket		obj hashͰ��С�����hash size���Ǻܴ󣬽���bucket��һ���ӽ������size��ֵ
 *		unsigned int timebucket timer hashͰ��С��ȱʡ60*1000*10�������10���ӳ�ʱ
 *
 * Return Value: 
 *
 * Note: ֻ֧��long int Ϊkey
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

	// ����hash�ڵ�����
	m_IdleList._head = NULL;
	m_IdleList._tail = NULL;

	// ��ʼ��hash�ڵ�
	for( unsigned int i=0; i<size; ++i)
	{
		_hash_obj *obj = new _hash_obj;

		if ( NULL == obj )
		{
			_log_func("CTimerThread::CTimerThread(): alloc memory: _hash_obj * failure!\n");
			break;
		}

		memset(obj, 0, sizeof(_hash_obj));

		// ��hash�ڵ�����������
		if ( 0 == i )
		{
			// ��һ�����нڵ�
			m_IdleList._head = obj;
			m_IdleList._tail = obj;
		}
		if ( NULL != m_IdleList._tail )
		{
			m_IdleList._tail->_next_obj = obj;
			m_IdleList._tail = obj;
		}
	}
	
	// ��ʼ������hash
	// ����hash
	m_ppObjHash = new _hash_obj*[m_nBucket];
	if ( NULL == m_ppObjHash )
	{
		_log_func("CTimerThread::CTimerThread(): alloc memory: m_ppObjHash failure!\n");
	}
	for( unsigned int i=0; i<m_nBucket; ++i)
	{
		m_ppObjHash[i] = NULL;
	}

	// ʱ��hash
	m_ppTimerHash = new _hash_obj* [m_nTimeBucket];
	if ( NULL == m_ppTimerHash )
	{
		_log_func("CTimerThread::CTimerThread(): alloc memory: m_ppTimerHash failure!\n");
	}
	for( unsigned int i=0; i<m_nTimeBucket; ++i)
	{
		m_ppTimerHash[i] = NULL;
	}

	// ��ѭ����־
	running = true;
}

/***************************************************************************
 * Function name: CTimerThread
 *
 * Description: TimerThread�͹�����
 *				������ѭ���˳����ͷ�hash�ڵ�
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
	// ���˳���־�����ȴ���ѭ���˳�
	running = false;
	usleep(10000);

	// �ͷ���hash�е�objָ�룬ֻ��Ҫ�ͷ�ObjHash��TimerHash����һ���Ϳ�����
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
 * Description: �����ݰ�����TimerHash�еȴ���ʱ����
 *
 * Parameters:  
 *		unsigned long key	���ݰ�key
 *		void *p				���ݰ�ָ��, obj hash��timer hash���������ݰ�����
 *		unsigned int msec	���ݰ��ĳ�ʱʱ��(����), =0 ��ʾʹ��default��ʱʱ��
 *
 * Return Value:  0		�ɹ�
 *				 -1		hashû�пռ�
 *
 * Note: ��֧����ͬkey�����
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
int CTimerThread::nPut(unsigned long key, void *p, unsigned int msec )
{
	// bucket
	int h;

	// 1.��ס��Դ
	m_lock.Lock();

	// 2. ����һ��δ�õ�hash�ڵ�
	_hash_obj *hobj = m_IdleList._head;

	// �Ѿ�û�п���hash�ڵ�
	if ( NULL == hobj )
	{
		m_lock.UnLock();
		return -1;
	}
	else
	{
		// ��һ�����нڵ��Ϊ�׽ڵ�
		m_IdleList._head = hobj->_next_obj;

		// ���ڵ��Ѿ����У�����ָ���κο��нڵ�
		hobj->_next_obj = NULL;

		// ����׽ڵ�Ϊ�գ���β�ڵ�Ҳһ��Ϊ��
		if ( NULL == m_IdleList._head )
			m_IdleList._tail = NULL;
	}

	hobj->obj = p;

	unsigned int _msec = msec;
	if ( 0 == _msec )
	{
		_msec = MacDefaultTimeout;
	}

	// ������TimerHash�е�bucketλ��
	hobj->_key_timer._key = (nGetMsec()+_msec)%m_nTimeBucket;
	hobj->_key_timer._prev = NULL;
	hobj->_key_timer._next = NULL;

	hobj->_key_obj._key = key;
	hobj->_key_obj._prev = NULL;
	hobj->_key_obj._next = NULL;

	// 3.�ȷ���ObjHash��
	// ������ObjHash�е�bucketλ��
	h =nHashFunc(key);

	// �׽ڵ�Ϊ��
	_hash_obj *obj = m_ppObjHash[h];
	if ( NULL == obj )
	{
		m_ppObjHash[h] = hobj;
	}
	else
	{
		// �׽ڵ�ǿգ���ŵ���β
		while( NULL != obj->_key_obj._next )
		{
			obj = obj->_key_obj._next;
		}

		// ���������νڵ�ָ��
		obj->_key_obj._next = hobj;
		hobj->_key_obj._prev = obj;
	}

	// 4.���ȷ���TimerHash��
	h = hobj->_key_timer._key;

	// �׽ڵ�Ϊ��
	obj = m_ppTimerHash[h];
	if ( NULL == obj )
	{	
		m_ppTimerHash[h] = hobj;
	}
	else
	{
		// �׽ڵ�ǿգ��ŵ���β
		while( NULL != obj->_key_timer._next )
		{
			obj = obj->_key_timer._next;
		}

		// ���������νڵ�ָ��
		obj->_key_timer._next = hobj;
		hobj->_key_timer._prev = obj;
	}

	m_lock.UnLock();

	return 0;
}

/***************************************************************************
 * Function name: pvGet
 *
 * Description: ��hash�и���keyȡ�����ݰ�ָ��
 * 				�ɹ���objhash��timerhash�еĽڵ�ᱻɾ��
 *
 * Parameters:  
 *		unsigned long key	���ݰ�key
 *
 * Return Value:  
 *		void *	not null, �������ݰ�ָ��
 *				null, û�и�key�����ݰ�ָ��
 *
 * Note: ��֧����ͬkey�����
 * 		 �����ݰ���������ʱ���ã���������ã���TimerThread�л��������ݰ����г�ʱ���п�������ҵ���߼�����
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------

 ***************************************************************************/
void *CTimerThread::pvGet(unsigned long key)
{
	void *p = NULL;

	// ��ס��Դ
	m_lock.Lock();

	int h = nHashFunc(key);

	// bucket�׽ڵ�
	_hash_obj *hobj  = m_ppObjHash[h];

	while( NULL != hobj )
	{
		// ��keyֵ��Ӧ�Ľڵ�
		if ( hobj->_key_obj._key == key )
		{
			// ����ObjHash�����νڵ�ָ��
			_hash_obj *up_obj = hobj->_key_obj._prev;
			_hash_obj *down_obj = hobj->_key_obj._next;
			if ( (NULL==up_obj) && (NULL==down_obj) )
			{
				// û�������νڵ㣬���ڵ�Ϊbucket�׽ڵ㣬��ֻ�б��ڵ�
				m_ppObjHash[h] = NULL;
			}
			else
			if ( (NULL!=up_obj) && (NULL!=down_obj) )
			{
				// �������νڵ�,�˽ڵ�Ϊ�м�ڵ�
				up_obj->_key_obj._next = down_obj;
				down_obj->_key_obj._prev = up_obj;
			}
			else
			if ( (NULL==up_obj) && (NULL!=down_obj) )
			{
				// û�����νڵ㣬�������νڵ㣬���ڵ�Ϊbucket�׽ڵ�
				// ���νڵ��Ϊ�׽ڵ�
				down_obj->_key_obj._prev = NULL;
				m_ppObjHash[h] = down_obj;
			}
			else
			if ( (NULL!=up_obj) && (NULL==down_obj) )
			{
				// �����νڵ㣬��û�����νڵ㣬���ڵ�Ϊbucketβ�ڵ�
				up_obj->_key_obj._next = NULL;
			}

			// ͬʱ����TimerHash�����νڵ�ָ��
			up_obj = hobj->_key_timer._prev;
			down_obj = hobj->_key_timer._next;
			if ( (NULL==up_obj) && (NULL==down_obj) )
			{
				// û�������νڵ㣬���ڵ�Ϊbucket�׽ڵ㣬��ֻ�б��ڵ�
				m_ppTimerHash[hobj->_key_timer._key] = NULL;
			}
			else
			if ( (NULL!=up_obj) && (NULL!=down_obj) )
			{
				// �������νڵ�,�˽ڵ�Ϊ�м�ڵ�
				up_obj->_key_timer._next = down_obj;
				down_obj->_key_timer._prev = up_obj;
			}
			else
			if ( (NULL==up_obj) && (NULL!=down_obj) )
			{
				// û�����νڵ㣬�������νڵ㣬���ڵ�Ϊbucket�׽ڵ�
				// ���νڵ��Ϊ�׽ڵ�
				down_obj->_key_timer._prev = NULL;
				m_ppTimerHash[hobj->_key_timer._key] = down_obj;
			}
			else
			if ( (NULL!=up_obj) && (NULL==down_obj) )
			{
				// �����νڵ㣬��û�����νڵ㣬���ڵ�Ϊbucketβ�ڵ�
				up_obj->_key_timer._next = NULL;
			}

			// ����obj����
			p = hobj->obj;


			// �ͷŵ�ǰhash�ڵ㣬���¹ҷſ��ж���
			// ���β�ڵ�ǿգ��򽫴�hobj�������������
			if ( NULL != m_IdleList._tail )
			{
				// ԭβ�ڵ����һ�����нڵ�ָ�򱾽ڵ�
				m_IdleList._tail->_next_obj = hobj;
				// ���ڵ��Ϊβ�ڵ�
				m_IdleList._tail = hobj;
			}
			else
			{
				// β�ڵ�Ϊ�գ�ͷ�ڵ�Ҳһ��Ϊ��
				m_IdleList._head = hobj;
				m_IdleList._tail = hobj;
			}

			break;
		}
		else
		{
			// ��һ���ڵ㣬ֱ��û��
			hobj = hobj->_key_obj._next;
		}
	}

	m_lock.UnLock();

	return p;
}


/***************************************************************************
 * Function name: nGetMsec
 *
 * Description: ȡ��ǰʱ��Ķ�ʱ����Ժ�����
 *
 * Parameters:  
 *
 * Return Value: 
 *		int		��ʱ����Ժ���������n�����ڵĵ�m����
 *
 * Note: ���tv_usec>500������4��5���������Ժ�����+1
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------
 
***************************************************************************/
inline int CTimerThread::nGetMsec()
{
 	struct timeval now;
	gettimeofday(&now, 0);

	// usec 4��5�������>500usecʱ����1msec
	int m = 0;
	if ( (now.tv_usec%1000) > 500 )
		m = 1;
		
	// ��ǰʱ��%600(��)��Ϊ10�����ڵ�����
	// (m_nTimeBucket/1000) = ���ʱ����
	return ((now.tv_sec%(m_nTimeBucket/1000))*1000+now.tv_usec/1000+m);
}


/***************************************************************************
 * Function name: nToMsec
 *
 * Description: �����Ժ�����תΪ��ʱ����Ժ�����
 *
 * Parameters:  
 *		TUINT64 nAbsMsec	���Ժ���������tv.tv_sec*1000+tv.tv_usec/1000
 *
 * Return Value: 
 *		int		��ʱ����Ժ���������n�����ڵĵ�m����
 *
 * Note: 
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------
 
***************************************************************************/
inline int CTimerThread::nToMsec(TUINT64 nAbsMsec)
{
	// (m_nTimeBucket/1000) = ���ʱ����
	return (int)(((nAbsMsec/1000)%(m_nTimeBucket/1000))*1000+nAbsMsec%1000);
}


/***************************************************************************
 * Function name: nCheckTimerHash
 *
 * Description: ��鳬ʱ�ڵ�
 *				����һ��(������)bucket��ʼɨ��TimerHash,ֱ������(������)bucketΪֹ��
 *              ÿ��bucket��ʾĳһ����(n���ں�����)����bucket�еĽڵ��ʾ�ڸú�����ʱ���ڵ�ᳬʱ
 *
 * Parameters:  
 *
 * Return Value:  
 *			0		�ɹ�
 *			-1		ʧ�ܣ�ObjHash��TimerHashΪ��
 *
 * Note: ��ʱ�ڵ㱾��ֻ����ҵ�����ݰ���ָ�룬TimerThread�׳���ʱ�ڵ����麯��vHandleTimeout����
 *       ��ʱ�Ľڵ���Դ�ᱻ�ͷŻ��գ���ObjHash��TimerHash��ͬʱɾ����������нڵ�����б�����ʹ��
 *
 * Modification Log:
 * DATE          AUTHOR           DESCRIPTION
 
 *--------------------------------------------------------------------------
 
***************************************************************************/
int CTimerThread::nCheckTimerHash()
{
	if ( (NULL == m_ppTimerHash) || (NULL == m_ppObjHash) )
		return -1;
		
	// ȡ��ǰʱ��(����)
	TUINT64 mSec = GetTime();

	// ���ϴ�nCheckTimerHash��(����)ʱ���
	int elapse = mSec - m_nLastMSec;

	// ��ס��Դ
	m_lock.Lock();

	// ����nUSleep���Ȳ��ߣ�������Ҫ���ϴ�m_nLastMSec��ʼ���TimerHash������æʱ�в���bucketδ����
	for(int i=1; i<=elapse; ++i)
	{
		// ������Ҫ����bucket
		// �ڴ�bucket�ڵ����нڵ㶼�Ѿ���ʱ
		int h = (nToMsec(m_nLastMSec)+i)%m_nTimeBucket;

		// bucket�׽ڵ�
		_hash_obj *hobj  = m_ppTimerHash[h];
		while( NULL != hobj )
		{
			// ����ʱ�ڵ��׳�����
			vHandleTimeout(hobj->obj);

			// ͬʱ����ObjHash�����νڵ�ָ��
			_hash_obj *up_obj = hobj->_key_obj._prev;
			_hash_obj *down_obj = hobj->_key_obj._next;
			int h1 = nHashFunc(hobj->_key_obj._key);
			if ( (NULL==up_obj) && (NULL==down_obj) )
			{
				// û�������νڵ㣬���ڵ�Ϊbucket�׽ڵ㣬��ֻ�б��ڵ�
				m_ppObjHash[h1] = NULL;
			}
			else
			if ( (NULL!=up_obj) && (NULL!=down_obj) )
			{
				// �������νڵ�,�˽ڵ�Ϊ�м�ڵ�
				up_obj->_key_obj._next = down_obj;
				down_obj->_key_obj._prev = up_obj;
			}
			else
			if ( (NULL==up_obj) && (NULL!=down_obj) )
			{
				// û�����νڵ㣬�������νڵ㣬���ڵ�Ϊbucket�׽ڵ�
				// ���νڵ��Ϊ�׽ڵ�
				down_obj->_key_obj._prev = NULL;
				m_ppObjHash[h1] = down_obj;
			}
			else
			if ( (NULL!=up_obj) && (NULL==down_obj) )
			{
				// �����νڵ㣬��û�����νڵ㣬���ڵ�Ϊbucketβ�ڵ�
				up_obj->_key_obj._next = NULL;
			}


			// �ͷŵ�ǰhash�ڵ㣬���¹�����ж���
			// ���β�ڵ�ǿգ��򽫴�hobj�������������
			if ( NULL != m_IdleList._tail )
			{
				// ԭβ�ڵ����һ�����нڵ�ָ�򱾽ڵ�
				m_IdleList._tail->_next_obj = hobj;
				// ���ڵ��Ϊβ�ڵ�
				m_IdleList._tail = hobj;
			}
			else
			{
				// β�ڵ�Ϊ�գ�ͷ�ڵ�Ҳһ��Ϊ��
				m_IdleList._head = hobj;
				m_IdleList._tail = hobj;
			}

			// ��bucket����һ���ڵ㣬ֱ��û��
			hobj = hobj->_key_timer._next;
		}

		// ��bucket�ڵ�ȫ���������׽ڵ�Ϊ��
		m_ppTimerHash[h] = NULL;
	}

	// �������һ�δ���ĺ�����
	m_nLastMSec = mSec;

	m_lock.UnLock();

	return 0;
}


/***************************************************************************
 * Function name: nDeleteAllObj 
 *
 * Description: ɾ������Hash�ڵ�
 *              Hash�ڵ�ֲ���ObjHash/TimerHash��IdleList��
 *		 		����ObjHash��TimerHash�ǹ���hash�ڵ㣬����ֻ��Ҫ����ɾ��ObjHash��TimerHash�е�����һ��
 *
 * Parameters:  
 *
 * Return Value:  
 *			>0		�ɹ�ɾ���ڵ�ĸ�����Ӧ��=hash size
 *
 * Note: ������ֻ��TimerThread�˳�ʱʹ��
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

	// ��������bucket
	for( unsigned int i=0; i< m_nBucket; ++i)
	{
		_hash_obj *hobj = m_ppObjHash[i];

		while ( NULL != hobj )
		{		
			_hash_obj *tobj = hobj->_key_obj._next;

			delete hobj;
			
			//ɾ���Ľڵ����
			++hash_count;
			
			hobj = tobj;
		}
	}

	// ������������
	_hash_obj *hobj = m_IdleList._head;
	while( NULL != hobj )
	{
		_hash_obj *tobj = hobj->_next_obj;

		delete hobj;
		//ɾ���Ľڵ����
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
 * Description: ��ʱ������
 *
 * Parameters:  
 *		void *p			�Ѿ���ʱ�����ݰ�
 *
 * Return Value:  
 *
 * Note: !!! ��������vHandleTimeout !!!
 *		 ���������߼�Ӧ�þ����ᣬ�����߼����ص���TimerThread�޷����������ĳ�ʱ�ڵ�
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
 * Description: hash����
 *				����key��hash�е�bucketֵ
 *
 * Parameters:  
 *		unsigned long key	���ݰ�key
 *
 * Return Value:  
 *		int 	ɢ�е�hash�е�bucketֵ
 *
 * Note: �������ش˺���
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
 * Description: �߳���ѭ��������
 *
 * Parameters:  
 *
 * Return Value:  
 *
 * Note: TimerThread�˳�ʱ����running������ѭ���˳�
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

		/* ���TimerHash�Ƿ��г�ʱ */
		nCheckTimerHash();
	}

	_log_func("CTimerThread::Run(): exit\n");
}

