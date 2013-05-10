#ifndef _TIMER_THREAD_H_
#define _TIMER_THREAD_H_

#include "lock.h"
#include "runable.h"
#include "sp_util.h"


struct _hash_obj;

struct _hash_key
{
	// obj.key��timer.key
	unsigned long _key;

	// ˫������
	// �����ǳ�ʱ����������ڵ�ʱ�������Կ��ٸ�������hash
	_hash_obj *_prev;
	_hash_obj *_next;
};

// hash�ڵ���Ϣ
struct _hash_obj
{
	// ���ݰ�ָ��
	void *obj;

	// �ڿ��ж���ʱ��ָ����һ�����нڵ�
	// û�ڿ��ж���ʱ������һ���ڵ�ΪNULL
	_hash_obj   *_next_obj;

	// һ���ڵ����ͨ��ʱ���key������
	_hash_key	_key_timer;		// ʱ��Ϊkey��hash
	_hash_key	_key_obj;		// obj.keyΪkey��hash
};

// ���ж���,  ������hash�е�_next_obj���
struct _list_obj
{
	_hash_obj *_head;	// ����ͷ�ڵ�ָ��
	_hash_obj *_tail;	// ����β�ڵ�ָ��
};

class CTimerThread : public CRunable
{
private:
	bool running;
	
private:
	// �߳���
	// ����û��ʹ��CAutoLock, ��ΪCAutoLock�����ܱ�CThreadLock�½�30%����
	// TimerThread��Ҫ������bucket�Ķ�λ�ϣ�CAutoLock��������ʧҲ���Կ��ǲ���
	CThreadLock m_lock;

	// m_ppTimerHash����Ժ�����Ϊbucketֵ
	// m_ppTimerHash[0]Ϊ��׼ʱ��ÿСʱ��0,10,20,30,40,50��0.000��
	// ��ÿһ��bucket��ʾ����10���ڵ���Ժ���
	// ��m_ppTimerHash[10000]��ʾ����10�����ڵĵ�10000����
	_hash_obj **m_ppTimerHash;

	// m_ppObjHash��key��hashɢ��
	_hash_obj **m_ppObjHash;

	// ����hash�ڵ����
	_list_obj m_IdleList;

	unsigned int m_nHashSize;	// hash��С
	unsigned int m_nBucket;		// hashͰ��
	unsigned int m_nTimeBucket;	// ʱ��hashͰ��

	TUINT64 m_nLastMSec;	// �ϴ�nCheckTimerHash�ĺ�����

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
	***************************************************************************/
	CTimerThread(unsigned int size, unsigned int bucket, unsigned int timebucket=60*1000*10ul);


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
	 *--------------------------------------------------------------------------
	***************************************************************************/
	~CTimerThread();


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
	***************************************************************************/
	int nPut(unsigned long key, void *p, unsigned int msec=0 );


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
	***************************************************************************/
	void *pvGet(unsigned long key);

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
	***************************************************************************/
	virtual void vHandleTimeout(void *p);

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
	***************************************************************************/
	virtual int nHashFunc(unsigned long key);

};

#endif
