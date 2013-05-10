/*
 * @File: mbsearch.h
 * @Desc: head file of Binary Search in Share Memory
 * @Author: stevetang
 * @History:
 *      2009-01-09   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _MBSEARCH_H
#define _MBSEARCH_H

#include "shm.h"

#pragma pack(1)

typedef struct _tagMBSearch {
	unsigned int node_total_; /* 节点总数 */
	unsigned int data_size_; /* 数据长度 */
	char data_[0]; /* 数据 */
} recMBSearch;

#pragma pack()

//键值比较 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pMBSearchKeyCmp)(const void *, const void *);

//键值比较 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pMBSearchDataCmp)(const void *, const void *);

class CMBSearch
{
	int m_shm_key; /* 共享内存键值 */
    CSVShm m_shm; /* 共享内存 */

	int m_data_size; /* 数据长度 */

	unsigned int m_cur; /* 当前节点数 */
	recMBSearch * m_rMBSearch; /* 数据信息 */

	static pMBSearchKeyCmp m_key_cmp; /* 键值比较 */
	static pMBSearchDataCmp m_data_cmp; /* 数据比较 */

public:
	CMBSearch(int shm_key, int data_size, pMBSearchKeyCmp key_cmp, pMBSearchDataCmp data_cmp);
	~CMBSearch();

public:
    /*
     * Function: open
     * Desc: 打开共享内存
     * In: 
     *     int                  shm_size   共享内存大小
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int open(int shm_size = 0, unsigned char init = 0);

    /*
     * Function: close
     * Desc: 关闭共享内存
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int close();

	/*
     * Function: pre_add
     * Desc: 计算节点数目
     * In: 
     *     void *   data       数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int pre_add(const void * data);

    /*
     * Function: pre_finish
     * Desc: 准备完成，分配数据空间
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int pre_finish();

    /*
     * Function: add
     * Desc: 新增数据
     * In: 
	 *     const void *   data        数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int add(const void * data);

    /*
     * Function: add_finish
     * Desc: 新增完成，进行排序
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int add_finish();

    /*
     * Function: get
     * Desc: 获取数据
     * In: 
     *     const void *   key         键值
     * Out: 
     *     void *         data        数据
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int get(const void * key, void * data);

	int get_shm_key() { return m_shm_key; };
	int get_total_node() { return m_rMBSearch->node_total_; };
	int get_data_size() { return m_rMBSearch->data_size_; };
	size_t get_data_addr() { return (size_t)m_rMBSearch->data_; };

	recMBSearch * get_search() const { return m_rMBSearch; };
};

#endif
