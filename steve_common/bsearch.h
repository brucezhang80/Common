/*
 * @File: bsearch.h
 * @Desc: head file of Binary Search
 * @Author: stevetang
 * @History:
 *      2009-01-09   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _BSEARCH_H
#define _BSEARCH_H

#pragma pack(1)

typedef struct _tagBSearch {
	unsigned int node_total_; /* 节点总数 */
	unsigned int data_size_; /* 数据长度 */
	char * data_; /* 数据 */
} recBSearch;

#pragma pack()

//键值比较 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pBSearchKeyCmp)(const void *, const void *);

//键值比较 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pBSearchDataCmp)(const void *, const void *);

class CBSearch
{
	unsigned int m_cur; /* 当前节点数 */
	recBSearch m_rBSearch; /* 数据信息 */

	pBSearchKeyCmp m_key_cmp; /* 键值比较 */
	static pBSearchDataCmp m_data_cmp; /* 数据比较 */

public:
	CBSearch(int data_size, pBSearchKeyCmp key_cmp, pBSearchDataCmp data_cmp);
	~CBSearch();

public:
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

	int get_total_node() { return m_rBSearch.node_total_; };
	int get_data_size() { return m_rBSearch.data_size_; };
	size_t get_data_addr() { return (size_t)m_rBSearch.data_; };
};

#endif
