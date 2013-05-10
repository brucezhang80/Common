/*
 * @File: hash.h
 * @Desc: head file of Hash
 * @Author: stevetang
 * @History:
 *      2008-11-07   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _HASH_H
#define _HASH_H

#include "baselist.h"
#include "lock.h"

class CHash;

class CHashNode {
	friend class CHash;

public:
	CHashNode();

	~CHashNode();

protected:
	size_t m_key; /* ��ֵ */
	void * m_obj; /* ��Ӧ���ݶ��� */
	size_t m_next; /* ��һ�ڵ�ָ�� */

public:
    /*
     * Function: Attach
     * Desc: �󶨶���
     * In: 
	 *      unsigned int  key   ��ֵ
     *      void *        obj   ����
	 *      unsigned int  next  ��һ�ڵ�
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int Attach(size_t key, void *obj, size_t next);

    /*
     * Function: Detach
     * Desc: �ͷŶ���
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int Detach();
};

class CHash {
	CThreadLock m_lock; /* �� */
	CGenericList<CHashNode> m_idle; /* ���нڵ��б� */
	unsigned int m_bcnt; /* Ͱ��Ŀ */
	size_t * m_bucket; /* Ͱ�б� */

	int m_num; /* �ڵ���Ŀ */

	CHashNode * m_cur; /* ��ǰö��NODE */
	int m_bcur; /* ��ǰö��Ͱ */

public:
	CHash(int num);
	~CHash();

private:
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
	unsigned GetBiggestPrime(unsigned maxNum);

public:
    /*
     * Function: Add
     * Desc: �����ڵ�
     * In: 
     *      unsigned int   key  ��ֵ
	 *      void *         obj  ����
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int Add(size_t key, void * obj);

    /*
     * Function: Remove
     * Desc: �Ƴ��ڵ�
     * In: 
     *      unsigned int   key  ��ֵ
     * Out: 
     *      none        
     * Return code: 
	 *      ��NULL  -   �ɹ�
	 *      NULL    -   ʧ��
     */
	void * Remove(size_t key);

	/*
     * Function: Get
     * Desc: ��ȡ�ڵ�����
     * In: 
     *      unsigned int   key  ��ֵ
     * Out: 
     *      none        
     * Return code: 
	 *      ��NULL  -   �ɹ�
	 *      NULL    -   ʧ��
     */
	void * Get(size_t key);

	/*
     * Function: RemoveHead
     * Desc: ���HASHͰ�е��׽ڵ�
     * In: 
     *      int   bucket   Ͱ�� -1��ʾ�Ƴ���һ���ǿսڵ�
     * Out: 
     *      none        
     * Return code: 
	 *      ��NULL  -   �ɹ�
	 *      NULL    -   ʧ��
     */
	void * RemoveHead(int& bucket);

	/*
     * Function: EnumBegin
     * Desc: ö�ٿ�ʼ
     * In: 
     *      none        
     * Out: 
     *      none        
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int EnumBegin();

	/*
     * Function: EnumNext
     * Desc: ö����һ���ڵ�
     * In: 
     *      none        
     * Out: 
     *      none        
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	void * EnumNext();

	/*
     * Function: EnumEnd
     * Desc: ö�ٽ���
     * In: 
     *      none        
     * Out: 
     *      none        
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int EnumEnd();

	int GetNodeNum() { return m_num; };
};

#endif
