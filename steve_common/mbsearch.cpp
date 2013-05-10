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

pMBSearchKeyCmp CMBSearch::m_key_cmp = NULL; /* ��ֵ�Ƚ� */
pMBSearchDataCmp CMBSearch::m_data_cmp = NULL; /* ���ݱȽ� */

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
 * Desc: �򿪹����ڴ�
 * In: 
 *     int                  shm_size   �����ڴ��С
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMBSearch::open(int shm_size /* = 0 */, unsigned char init /* = 0 */)
{
	/* �򿪹����ڴ� */
	int ret = m_shm.open_and_attach((key_t)m_shm_key, shm_size);
	if(ret < 0 && shm_size == 0)
	{
		/* �����ڴ治���ڣ���Ϊ�Ǵ����� */
		return -1;
	}
	else if (ret < 0)
	{
		/* �����ڴ治���ڻ򳤶Ȳ��ԣ����´�/���������ڴ� */
		int ret = m_shm.force_open_and_attach((key_t)m_shm_key, (size_t)shm_size, init);
		if(ret < 0)
		{
			return -1;
		}

		/* ��ȡ����ָ�� */
		m_rMBSearch = (recMBSearch *)m_shm.get_segment_ptr();

		/* ���� */
		memset((char *)m_rMBSearch, 0, shm_size);

		m_rMBSearch->node_total_ = 0;
		m_rMBSearch->data_size_ = m_data_size;
	}
	else
	{
		/* �����ڴ���ڣ��Ҵ�С��ͬ����ȡ����ָ�� */
		m_rMBSearch = (recMBSearch *)m_shm.get_segment_ptr();

		if (shm_size && init)
		{
			/* ���� */
			memset((char *)m_rMBSearch, 0, shm_size);

			m_rMBSearch->node_total_ = 0;
			m_rMBSearch->data_size_ = m_data_size;
		}
	}

	return 0;
}

/*
 * Function: close
 * Desc: �رչ����ڴ�
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMBSearch::close()
{
	m_shm.detach();

	return 0;
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
int CMBSearch::pre_add(const void * data)
{
	m_rMBSearch->node_total_++;

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
int CMBSearch::pre_finish()
{
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
 * Desc: ������ɣ���������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMBSearch::add_finish()
{
	/* ���� */
	qsort(m_rMBSearch->data_, m_rMBSearch->node_total_, m_rMBSearch->data_size_, CMBSearch::m_data_cmp);

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
int CMBSearch::get(const void * key, void * data)
{
	char * ptr = m_rMBSearch->data_;

	/* �����ֲ��� */
	int low = 0;
	int high = m_rMBSearch->node_total_ - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * m_rMBSearch->data_size_;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* ��ֵ��ͬ */
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
