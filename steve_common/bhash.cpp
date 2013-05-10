/*
 * @File: bhash.cpp
 * @Desc: implement file of Binary-Hash
 * @Author: stevetang
 * @History:
 *      2008-12-24   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "bhash.h"

pBHashGetBucketId CBHash::m_get_bucket_id = NULL; /* ȡͰID */
pBHashKeyCmp CBHash::m_key_cmp = NULL; /* ��ֵ�Ƚ� */
pBHashDataCmp CBHash::m_data_cmp = NULL; /* ����ֵ�Ƚ� */
pBHashChkData CBHash::m_chk_data = NULL; /* У��������Ч�� */

CBHash::CBHash(int bucket_size, int data_size, pBHashGetBucketId get_bucket_id, pBHashKeyCmp key_cmp, pBHashDataCmp data_cmp, pBHashChkData chk_data)
{
	m_get_bucket_id = get_bucket_id;
	m_key_cmp = key_cmp;
	m_data_cmp = data_cmp;
	m_chk_data = chk_data;

	memset(&m_rBHash, 0, sizeof(recBHash));

    m_rBHash.bucket_size_ = bucket_size;  //HASHͰ�Ĵ�С
	m_rBHash.data_size_ = data_size;     //���ݳ���

	/* ����Ͱ�ռ� */
	m_rBHash.bucket_ = (recBHashBucket *)malloc(sizeof(recBHashBucket) * bucket_size); /* Ͱ */
	if (m_rBHash.bucket_ == NULL)
		return;

	memset(m_rBHash.bucket_, 0, sizeof(recBHashBucket) * bucket_size);
}

CBHash::CBHash(const CBHash * hash)
{
	m_get_bucket_id = hash->m_get_bucket_id;
	m_key_cmp = hash->m_key_cmp;
	m_data_cmp = hash->m_data_cmp;
	m_chk_data = hash->m_chk_data;

	memset(&m_rBHash, 0, sizeof(recBHash));

	m_rBHash.node_total_ = hash->m_rBHash.node_total_; /* �ڵ����� */
    m_rBHash.bucket_size_ = hash->m_rBHash.bucket_size_;  //HASHͰ�Ĵ�С
	m_rBHash.data_size_ = hash->m_rBHash.data_size_;     //���ݳ���

	/* ����Ͱ�ռ� */
	m_rBHash.bucket_ = (recBHashBucket *)malloc(sizeof(recBHashBucket) * m_rBHash.bucket_size_); /* Ͱ */
	if (m_rBHash.bucket_ == NULL)
		return;
	/* ����BUCKET���� */
	memcpy(m_rBHash.bucket_, hash->m_rBHash.bucket_, sizeof(recBHashBucket) * m_rBHash.bucket_size_);

	/* �������ݿռ� */
	m_rBHash.data_ = (char *)malloc(m_rBHash.node_total_ * m_rBHash.data_size_);
	if (m_rBHash.data_ == NULL)
		return;
	/* ������������ */
	memcpy(m_rBHash.data_, hash->m_rBHash.data_, m_rBHash.node_total_ * m_rBHash.data_size_);
}

CBHash::~CBHash()
{
	/* �ͷ���Դ */
	if (m_rBHash.bucket_)
	{
		free(m_rBHash.bucket_);
		m_rBHash.bucket_ = NULL;
	}

	if (m_rBHash.data_)
	{
		free(m_rBHash.data_);
		m_rBHash.data_ = NULL;
	}
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
int CBHash::pre_add(const void * key)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recBHashBucket * bucket = m_rBHash.bucket_ + id;

	if (bucket->num >= 0xFF)
		return -1;

	bucket->num++; /* ͬһͰ�г�ͻ��+1 */

	m_rBHash.node_total_++; /* �ܽڵ���+1 */

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
int CBHash::pre_finish()
{
	/* �������ݿռ� */
	m_rBHash.data_ = (char *)malloc(m_rBHash.node_total_ * get_data_size());
	if (m_rBHash.data_ == NULL)
		return -1;

	size_t offset = 0;
	for (int i=0; i<get_bucket_size(); i++)
	{
		recBHashBucket * bucket = m_rBHash.bucket_ + i;

		if (bucket->num)
		{
			/* Ͱ��Ӧ��ʼ��ַ */
			bucket->addr = offset;

			/* ��һ��Ͱ��ʼ��ַ */
			offset += bucket->num * get_data_size();

			/* �建�� */
			size_t addr = get_node_addr() + bucket->addr;
			memset((char *)addr, 0, bucket->num * get_data_size());
		}
	}

	return 0;
}

/*
 * Function: add
 * Desc: ��������
 * In: 
 *     const void *   key        ��ֵ
 *     const void *   data        ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBHash::add(const void * key, const void * data)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recBHashBucket * bucket = m_rBHash.bucket_ + id;

	char * ptr = (char *)(bucket->addr + get_node_addr());
	for (int i=0; i<bucket->num; i++)
	{
		if (!m_chk_data(ptr))
		{
			/* ������� */
			memcpy(ptr, data, get_data_size());
			return 0;
		}

		ptr += get_data_size();
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
int CBHash::add_finish()
{
	/* ���ݵ�ַ */
	size_t addr = get_node_addr();
	for (int i=0; i<get_bucket_size(); i++)
	{
		recBHashBucket * bucket = m_rBHash.bucket_ + i;

		if (bucket->num > 0)
		{
			/* ���� */
			qsort((void *)(bucket->addr + addr), bucket->num, get_data_size(), CBHash::m_data_cmp);
		}
	}

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
int CBHash::get(const void * key, void * data)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recBHashBucket * bucket = m_rBHash.bucket_ + id;

	if (bucket->num <= 0)
		/* Ͱ�в����ڽڵ� */
		return -1;

	char * ptr = (char *)(bucket->addr + get_node_addr());

	/* �����ֲ��� */
	int low = 0;
	int high = bucket->num - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * get_data_size();

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* ��ֵ��ͬ */
		{
			memcpy(data, addr, get_data_size());
			return 0;
		}

		if (ret > 0)
			high = mid - 1;
		else
			low = mid + 1;
	}

	return -1;
}
