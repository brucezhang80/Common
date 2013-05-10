/*
 * @File: bsearch.cpp
 * @Desc: impliment file of Binary Search
 * @Author: stevetang
 * @History:
 *      2009-01-09   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "bsearch.h"

pBSearchDataCmp CBSearch::m_data_cmp = NULL; /* 数据比较 */

CBSearch::CBSearch(int data_size, pBSearchKeyCmp key_cmp, pBSearchDataCmp data_cmp) : 
	m_cur(0), m_key_cmp(key_cmp)
{
	m_rBSearch.node_total_ = 0;
	m_rBSearch.data_size_ = data_size;
	m_rBSearch.data_ = NULL;
	
	m_data_cmp = data_cmp;
}

CBSearch::~CBSearch()
{
	/* 释放资源 */
	if (m_rBSearch.data_)
	{
		free(m_rBSearch.data_);
		m_rBSearch.data_ = NULL;
	}
}

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
int CBSearch::pre_add(const void * data)
{
	m_rBSearch.node_total_++;

	return 0;
}

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
int CBSearch::pre_finish()
{
	m_rBSearch.data_ = (char *)malloc(m_rBSearch.node_total_ * m_rBSearch.data_size_);
	if (m_rBSearch.data_ == NULL)
		return -1;

	m_cur = 0;

	return 0;
}

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
int CBSearch::add(const void * data)
{
	if (m_cur >= m_rBSearch.node_total_)
		return -1;

	char * ptr = m_rBSearch.data_ + m_cur * m_rBSearch.data_size_;

	memcpy(ptr, data, m_rBSearch.data_size_);

	m_cur++;

	return 0;
}

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
int CBSearch::add_finish()
{
	/* 排序 */
	qsort(m_rBSearch.data_, m_rBSearch.node_total_, m_rBSearch.data_size_, m_data_cmp);

	return 0;
}

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
int CBSearch::get(const void * key, void * data)
{
	char * ptr = m_rBSearch.data_;

	/* 做二分查找 */
	int low = 0;
	int high = m_rBSearch.node_total_ - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * m_rBSearch.data_size_;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* 键值相同 */
		{
			memcpy(data, addr, m_rBSearch.data_size_);
			return 0;
		}

		if (ret > 0)
			high = mid - 1;
		else
			low = mid + 1;
	}

	return -1;
}
