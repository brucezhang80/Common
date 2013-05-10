/*
 * @File: hash.cpp
 * @Desc: impliment file of Hash
 * @Author: stevetang
 * @History:
 *      2008-11-07   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include <math.h>
#include "general.h"
#include "hash.h"

CHashNode::CHashNode()
{ 
	m_key = 0;
	m_obj = NULL;
	m_next = 0;
}

CHashNode::~CHashNode()
{
	Detach();
}

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
int CHashNode::Attach(size_t key, void *obj, size_t next)
{
	m_key = key;
	m_obj = obj;
	m_next = next;

	return 0;
}

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
int CHashNode::Detach() 
{
	m_key = 0;
	m_obj = 0;
	m_next = 0;

	return 0;
}

CHash::CHash(int num) : m_idle(num), m_num(0), m_cur(NULL), m_bcur(0)
{
	/* ׼�����нڵ� */
	for (int i=0; i<num; i++)
	{
		CHashNode * node = new CHashNode();
		m_idle.AddTail(node);
	}

	/* ȷ��Ͱ�� */
	m_bcnt = GetBiggestPrime(num);
	if (m_bcnt == 0)
		m_bcnt = 7; /* ����7��Ͱ */

	m_bucket = (size_t *)malloc(m_bcnt * sizeof(size_t));
	if (m_bucket == NULL)
		return;
	memset(m_bucket, 0, sizeof(size_t) * m_bcnt);
}

CHash::~CHash() 
{
	CAutoLock l(&m_lock);

	CHashNode * node = NULL;

	/* �ͷſ��нڵ�� */
	while (1)
	{
		node = m_idle.RemoveHead();
		if (node == NULL)
			break;
		delete node;
	}

	if (m_bucket == NULL)
		return;

	/* �ͷ�HASH�� */
	for (int i=0; i<(int)m_bcnt; i++)
	{
		node = (CHashNode *)m_bucket[i];
		while (node)
		{
			CHashNode * next = (CHashNode *)node->m_next;
			delete node;
			node = next;
		}
	}

	free(m_bucket);
	m_bucket = NULL;
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
unsigned CHash::GetBiggestPrime(unsigned maxNum)
{	
	//��ȡ��maxNum��ӽ����������
	//������
	// ...
		
	//������ָ�������������
	unsigned i = 0;
	for(i=maxNum;i>=2;i--)
	{
		unsigned end=(unsigned)(sqrt((double)i)+1);
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
int CHash::Add(size_t key, void * obj)
{
	CAutoLock l(&m_lock);

	/* ȡ�ÿ��нڵ� */
	CHashNode * node = m_idle.RemoveHead();
	if (node == NULL)
		return -1;

	/* ��ȡͰ�ź�ͷ��ַ */
	unsigned int site = key % m_bcnt;
	size_t next = m_bucket[site];

	/* �󶨵��ڵ� */
	node->Attach(key, obj, next);

	/* �޸�ͷ��� */
	m_bucket[site] = (size_t)node;

	/* �ڵ���+1 */
	m_num++;

	return 0;
}

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
void * CHash::Remove(size_t key)
{
	CAutoLock l(&m_lock);

	/* ��ȡͰ�� */
	unsigned int site = key % m_bcnt;

	/* ȡ�ö�ӦͰͷ�ڵ� */
	CHashNode * node = (CHashNode *)m_bucket[site];
	if (node == NULL)
		return NULL;

	CHashNode * prev = node;
	while (node && node->m_key != key)
	{
		prev = node;
		node = (CHashNode *)node->m_next;
	}

	if (node == NULL)
		return NULL;

	if (prev == node) /* ͷ�ڵ� */
		m_bucket[site] = node->m_next;
	else /* �м�ڵ� */
		prev->m_next = node->m_next;

	void * obj = node->m_obj;

	node->Detach();
	m_idle.AddTail(node);

	/* �ڵ���-1 */
	m_num--;

	return obj;
}

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
void * CHash::Get(size_t key)
{
	CAutoLock l(&m_lock);

	/* ��ȡͰ�� */
	unsigned int site = key % m_bcnt;

	/* ȡ�ö�ӦͰͷ�ڵ� */
	CHashNode * node = (CHashNode *)m_bucket[site];
	if (node == NULL)
		return NULL;

	while (node && node->m_key != key)
	{
		node = (CHashNode *)node->m_next;
	}

	return (node == NULL) ? NULL : node->m_obj;
}

/*
 * Function: RemoveHead
 * Desc: ���HASHͰ�е��׽ڵ�
 * In: 
 *      none
 * Out: 
 *      none        
 * Return code: 
 *      ��NULL  -   �ɹ�
 *      NULL    -   ʧ��
 */
void * CHash::RemoveHead(int& bucket)
{
	CAutoLock l(&m_lock);

	void * obj = NULL;

	if (bucket >= 0)
	{
		CHashNode * node = (CHashNode *)m_bucket[bucket % m_bcnt];
		if (node)
		{
			obj = node->m_obj;
		}
	}
	else
	{
		for (int i=0; i<(int)m_bcnt; i++)
		{
			CHashNode * node = (CHashNode *)m_bucket[i];
			if (node == NULL)
				continue;

			m_bucket[i] = node->m_next;
			obj = node->m_obj;
			bucket = i;

			node->Detach();
			m_idle.AddTail(node);

			break;
		}
	}

	return obj;
}

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
int CHash::EnumBegin()
{
	/* ���� */
	m_lock.Lock();

	/* �ҵ���һ���ǿսڵ� */
	for (int i=0; i<(int)m_bcnt; i++)
	{
		CHashNode * node = (CHashNode *)m_bucket[i];
		if (node == NULL)
			continue;

		m_cur = node;
		m_bcur = i;
		break;
	}

	return 0;
}

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
void * CHash::EnumNext()
{
	if (m_cur == NULL)
		return NULL;

	void * obj = m_cur->m_obj;
	m_cur = (CHashNode *)m_cur->m_next;
	if (m_cur == NULL)
	{
		/* ��Ͱ�����,�ҵ���һ���ǿսڵ� */
		for (int i=m_bcur+1; i<(int)m_bcnt; i++)
		{
			CHashNode * node = (CHashNode *)m_bucket[i];
			if (node == NULL)
				continue;

			m_cur = node;
			m_bcur = i;
			break;
		}
	}

	return obj;
}

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
int CHash::EnumEnd()
{
	m_cur = NULL;
	m_bcur = 0;

	/* ���� */
	m_lock.UnLock();

	return 0;
}
