/*
 * @File: mbsearch.cpp
 * @Desc: impliment file of Binary Search in Share Memory
 * @Author: stevetang
 * @History:
 *      2009-01-09   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "mbsearch.h"

pMBSearchKeyCmp CMBSearch::m_key_cmp = NULL; /* 键值比较 */
pMBSearchDataCmp CMBSearch::m_data_cmp = NULL; /* 数据比较 */

CMBSearch::CMBSearch(int shm_key, int data_size, pMBSearchKeyCmp key_cmp, pMBSearchDataCmp data_cmp) : 
	m_shm_key(shm_key), m_data_size(data_size), m_cur(0), m_rMBSearch(NULL)
{
	m_key_cmp = key_cmp;
	m_data_cmp = data_cmp;
}

CMBSearch::~CMBSearch()
{
}

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
int CMBSearch::open(int shm_size /* = 0 */, unsigned char init /* = 0 */)
{
	/* 打开共享内存 */
	int ret = m_shm.open_and_attach((key_t)m_shm_key, shm_size);
	if(ret < 0 && shm_size == 0)
	{
		/* 共享内存不存在，且为非创建者 */
		return -1;
	}
	else if (ret < 0)
	{
		/* 共享内存不存在或长度不对，重新打开/创建共享内存 */
		int ret = m_shm.force_open_and_attach((key_t)m_shm_key, (size_t)shm_size, init);
		if(ret < 0)
		{
			return -1;
		}

		/* 获取数据指针 */
		m_rMBSearch = (recMBSearch *)m_shm.get_segment_ptr();

		/* 清零 */
		memset((char *)m_rMBSearch, 0, shm_size);

		m_rMBSearch->node_total_ = 0;
		m_rMBSearch->data_size_ = m_data_size;
	}
	else
	{
		/* 共享内存存在，且大小相同，获取数据指针 */
		m_rMBSearch = (recMBSearch *)m_shm.get_segment_ptr();

		if (shm_size && init)
		{
			/* 清零 */
			memset((char *)m_rMBSearch, 0, shm_size);

			m_rMBSearch->node_total_ = 0;
			m_rMBSearch->data_size_ = m_data_size;
		}
	}

	return 0;
}

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
int CMBSearch::close()
{
	m_shm.detach();

	return 0;
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
int CMBSearch::pre_add(const void * data)
{
	m_rMBSearch->node_total_++;

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
int CMBSearch::pre_finish()
{
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
int CMBSearch::add(const void * data)
{
	if (m_cur >= m_rMBSearch->node_total_)
		return -1;

	char * ptr = m_rMBSearch->data_ + m_cur * m_rMBSearch->data_size_;

	memcpy(ptr, data, m_rMBSearch->data_size_);

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
int CMBSearch::add_finish()
{
	/* 排序 */
	qsort(m_rMBSearch->data_, m_rMBSearch->node_total_, m_rMBSearch->data_size_, CMBSearch::m_data_cmp);

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
int CMBSearch::get(const void * key, void * data)
{
	char * ptr = m_rMBSearch->data_;

	/* 做二分查找 */
	int low = 0;
	int high = m_rMBSearch->node_total_ - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * m_rMBSearch->data_size_;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* 键值相同 */
		{
			memcpy(data, addr, m_rMBSearch->data_size_);
			return 0;
		}

		if (ret > 0)
			high = mid - 1;
		else
			low = mid + 1;
	}

	return -1;
}
