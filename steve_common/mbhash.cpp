/*
 * @File: mbhash.cpp
 * @Desc: implement file of Binary-Hash
 * @Author: stevetang
 * @History:
 *      2008-12-04   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "mbhash.h"

pMBHashGetBucketId CMBHash::m_get_bucket_id = NULL; /* ȡͰID */
pMBHashKeyCmp CMBHash::m_key_cmp = NULL; /* ��ֵ�Ƚ� */
pMBHashDataCmp CMBHash::m_data_cmp = NULL; /* ���ݱȽ� */
pMBHashChkData CMBHash::m_chk_data = NULL; /* У��������Ч�� */

CMBHash::CMBHash(int shm_key, int bucket_size, int data_size, pMBHashGetBucketId get_bucket_id, 
    pMBHashKeyCmp key_cmp, pMBHashDataCmp data_cmp, pMBHashChkData chk_data) : 
	m_shm_key(shm_key), m_hash(NULL), m_bucket_size(bucket_size), m_data_size(data_size)
{
	m_get_bucket_id = get_bucket_id;
	m_key_cmp = key_cmp;
	m_data_cmp = data_cmp;
	m_chk_data = chk_data;
}

CMBHash::~CMBHash()
{
	close();
}

/*
 * Function: open
 * Desc: ��MAP
 * In: 
 *     int                   shm_size   �����ڴ��С
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMBHash::open(int shm_size /* = 0 */, unsigned char init /* = 0 */)
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

		/* ����hash�ṹ */
		m_hash = (recMBHash *)m_shm.get_segment_ptr();

		/* ���� */
		memset((char *)m_hash, 0, shm_size);

		/* ����HASH��Ϣ */
		m_hash->node_total_ = 0;
		m_hash->bucket_size_ = m_bucket_size;
		m_hash->data_size_ = m_data_size;
	}
	else
	{
		/* �����ڴ���ڣ��Ҵ�С��ͬ����hash�ṹ�� */
		m_hash = (recMBHash *)m_shm.get_segment_ptr();

		if (shm_size && init)
		{
			/* ���� */
			memset((char *)m_hash, 0, shm_size);

			/* ����HASH��Ϣ */
			m_hash->node_total_ = 0;
			m_hash->bucket_size_ = m_bucket_size;
			m_hash->data_size_ = m_data_size;
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
int CMBHash::close()
{
	m_shm.detach();

	return 0;
}

/*
 * Function: pre_add
 * Desc: ����׼��,����ÿ��Ͱ��ͻ��Ŀ
 * In: 
 *     void *   key        ��ֵ
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMBHash::pre_add(const void * key)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recMBHashBucket * bucket = m_hash->bucket_ + id;

	if (bucket->num >= 0xFF)
		return -1;

	bucket->num++; /* ͬһͰ�г�ͻ��+1 */

	m_hash->node_total_++; /* �ܽڵ���+1 */

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
int CMBHash::pre_finish()
{
	size_t offset = 0; /* ����ƫ�Ƶ�ַ */

	for (int i=0; i<m_bucket_size; i++)
	{
		recMBHashBucket * bucket = m_hash->bucket_ + i;

		if (bucket->num)
		{
			/* Ͱ��Ӧ��ʼ��ַ */
			bucket->addr = offset;

			/* ��һ��Ͱ��ʼ��ַ */
			offset += bucket->num * m_data_size;

			/* �建�� */
			size_t addr = get_node_addr() + bucket->addr;
			memset((char *)addr, 0, bucket->num * m_data_size);
		}
	}

	return 0;
}

/*
 * Function: add
 * Desc: ��������
 * In: 
 *     void *   data        ����
 *     int      data_size   ���ݳ���
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMBHash::add(const void * key, const void * data)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recMBHashBucket * bucket = m_hash->bucket_ + id;

	/* ���ݵ�ַ */
	char * ptr = (char *)(bucket->addr + get_node_addr());

	for (int i=0; i<bucket->num; i++)
	{
		if (!m_chk_data(ptr))
		{
			/* ������� */
			memcpy(ptr, data, m_data_size);
			return 0;
		}

		ptr += m_data_size;
	}

	return -1;
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
int CMBHash::add_finish()
{
	/* ���ݵ�ַ */
	size_t addr = get_node_addr();
	for (int i=0; i<m_bucket_size; i++)
	{
		recMBHashBucket * bucket = m_hash->bucket_ + i;

		if (bucket->num > 0)
		{
			/* ���� */
			qsort((void *)(bucket->addr + addr), bucket->num, m_data_size, CMBHash::m_data_cmp);
		}
	}

	return 0;
}

/*
 * Function: get
 * Desc: ��ȡ����
 * In: 
 *     void *   data        ��ֵ
 * Out: 
 *     void *   data        ����
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMBHash::get(const void * key, void * data)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recMBHashBucket * bucket = m_hash->bucket_ + id;

	/* ���ݵ�ַ */
	char * ptr = (char *)(bucket->addr + get_node_addr());

	/* �����ֲ��� */
	int low = 0;
	int high = bucket->num - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * m_data_size;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* ��ֵ��ͬ */
		{
			memcpy(data, addr, m_data_size);
			return 0;
		}

		if (ret > 0)
			high = mid - 1;
		else
			low = mid + 1;
	}

	return -1;
}
