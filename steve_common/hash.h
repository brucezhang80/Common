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
	size_t m_key; /* 键值 */
	void * m_obj; /* 对应数据对象 */
	size_t m_next; /* 下一节点指针 */

public:
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
	int Attach(size_t key, void *obj, size_t next);

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
	int Detach();
};

class CHash {
	CThreadLock m_lock; /* 锁 */
	CGenericList<CHashNode> m_idle; /* 空闲节点列表 */
	unsigned int m_bcnt; /* 桶数目 */
	size_t * m_bucket; /* 桶列表 */

	int m_num; /* 节点数目 */

	CHashNode * m_cur; /* 当前枚举NODE */
	int m_bcur; /* 当前枚举桶 */

public:
	CHash(int num);
	~CHash();

private:
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
	unsigned GetBiggestPrime(unsigned maxNum);

public:
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
	int Add(size_t key, void * obj);

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
	void * Remove(size_t key);

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
	void * Get(size_t key);

	/*
     * Function: RemoveHead
     * Desc: 清除HASH桶中的首节点
     * In: 
     *      int   bucket   桶号 -1表示移除第一个非空节点
     * Out: 
     *      none        
     * Return code: 
	 *      非NULL  -   成功
	 *      NULL    -   失败
     */
	void * RemoveHead(int& bucket);

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
	int EnumBegin();

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
	void * EnumNext();

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
	int EnumEnd();

	int GetNodeNum() { return m_num; };
};

#endif
