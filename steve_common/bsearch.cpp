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

pBSearchDataCmp CBSearch::m_data_cmp = NULL; /* ���ݱȽ� */

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
	/* �ͷ���Դ */
	if (m_rBSearch.data_)
	{
		free(m_rBSearch.data_);
		m_rBSearch.data_ = NULL;
	}
}

/*
 * Function: pre_add
 * Desc: ����ڵ���Ŀ
 * In: 
 *     void *   data       ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBSearch::pre_add(const void * data)
{
	m_rBSearch.node_total_++;

	return 0;
}

/*
 * Function: pre_finish
 * Desc: ׼����ɣ��������ݿռ�
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
 * Desc: ��������
 * In: 
 *     const void *   data        ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
 * Desc: ������ɣ���������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBSearch::add_finish()
{
	/* ���� */
	qsort(m_rBSearch.data_, m_rBSearch.node_total_, m_rBSearch.data_size_, m_data_cmp);

	return 0;
}

/*
 * Function: get
 * Desc: ��ȡ����
 * In: 
 *     const void *   key         ��ֵ
 * Out: 
 *     void *         data        ����
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBSearch::get(const void * key, void * data)
{
	char * ptr = m_rBSearch.data_;

	/* �����ֲ��� */
	int low = 0;
	int high = m_rBSearch.node_total_ - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * m_rBSearch.data_size_;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* ��ֵ��ͬ */
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
