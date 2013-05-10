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
 * Desc: 绑定对象
 * In: 
 *      unsigned int  key   键值
 *      void *        obj   对象
 *      unsigned int  next  下一节点
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
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
 * Desc: 释放对象
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
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
	/* 准备空闲节点 */
	for (int i=0; i<num; i++)
	{
		CHashNode * node = new CHashNode();
		m_idle.AddTail(node);
	}

	/* 确定桶数 */
	m_bcnt = GetBiggestPrime(num);
	if (m_bcnt == 0)
		m_bcnt = 7; /* 最少7个桶 */

	m_bucket = (size_t *)malloc(m_bcnt * sizeof(size_t));
	if (m_bucket == NULL)
		return;
	memset(m_bucket, 0, sizeof(size_t) * m_bcnt);
}

CHash::~CHash() 
{
	CAutoLock l(&m_lock);

	CHashNode * node = NULL;

	/* 释放空闲节点表 */
	while (1)
	{
		node = m_idle.RemoveHead();
		if (node == NULL)
			break;
		delete node;
	}

	if (m_bucket == NULL)
		return;

	/* 释放HASH表 */
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
unsigned CHash::GetBiggestPrime(unsigned maxNum)
{	
	//获取离maxNum最接近的最大质数
	//检查参数
	// ...
		
	//测试离指定数最近的质数
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
 * Desc: 新增节点
 * In: 
 *      unsigned int   key  键值
 *      void *         obj  数据
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CHash::Add(size_t key, void * obj)
{
	CAutoLock l(&m_lock);

	/* 取得空闲节点 */
	CHashNode * node = m_idle.RemoveHead();
	if (node == NULL)
		return -1;

	/* 获取桶号和头地址 */
	unsigned int site = key % m_bcnt;
	size_t next = m_bucket[site];

	/* 绑定到节点 */
	node->Attach(key, obj, next);

	/* 修改头结点 */
	m_bucket[site] = (size_t)node;

	/* 节点数+1 */
	m_num++;

	return 0;
}

/*
 * Function: Remove
 * Desc: 移除节点
 * In: 
 *      unsigned int   key  键值
 * Out: 
 *      none        
 * Return code: 
 *      非NULL  -   成功
 *      NULL    -   失败
 */
void * CHash::Remove(size_t key)
{
	CAutoLock l(&m_lock);

	/* 获取桶号 */
	unsigned int site = key % m_bcnt;

	/* 取得对应桶头节点 */
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

	if (prev == node) /* 头节点 */
		m_bucket[site] = node->m_next;
	else /* 中间节点 */
		prev->m_next = node->m_next;

	void * obj = node->m_obj;

	node->Detach();
	m_idle.AddTail(node);

	/* 节点数-1 */
	m_num--;

	return obj;
}

/*
 * Function: Get
 * Desc: 获取节点数据
 * In: 
 *      unsigned int   key  键值
 * Out: 
 *      none
 * Return code: 
 *      非NULL  -   成功
 *      NULL    -   失败
 */
void * CHash::Get(size_t key)
{
	CAutoLock l(&m_lock);

	/* 获取桶号 */
	unsigned int site = key % m_bcnt;

	/* 取得对应桶头节点 */
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
 * Desc: 清除HASH桶中的首节点
 * In: 
 *      none
 * Out: 
 *      none        
 * Return code: 
 *      非NULL  -   成功
 *      NULL    -   失败
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
 * Desc: 枚举开始
 * In: 
 *      none        
 * Out: 
 *      none        
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CHash::EnumBegin()
{
	/* 加锁 */
	m_lock.Lock();

	/* 找到第一个非空节点 */
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
 * Desc: 枚举下一个节点
 * In: 
 *      none        
 * Out: 
 *      none        
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
void * CHash::EnumNext()
{
	if (m_cur == NULL)
		return NULL;

	void * obj = m_cur->m_obj;
	m_cur = (CHashNode *)m_cur->m_next;
	if (m_cur == NULL)
	{
		/* 本桶已完成,找到下一个非空节点 */
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
 * Desc: 枚举结束
 * In: 
 *      none        
 * Out: 
 *      none        
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CHash::EnumEnd()
{
	m_cur = NULL;
	m_bcur = 0;

	/* 解锁 */
	m_lock.UnLock();

	return 0;
}
