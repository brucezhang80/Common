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

pBHashGetBucketId CBHash::m_get_bucket_id = NULL; /* 取桶ID */
pBHashKeyCmp CBHash::m_key_cmp = NULL; /* 键值比较 */
pBHashDataCmp CBHash::m_data_cmp = NULL; /* 数据值比较 */
pBHashChkData CBHash::m_chk_data = NULL; /* 校验数据有效性 */

CBHash::CBHash(int bucket_size, int data_size, pBHashGetBucketId get_bucket_id, pBHashKeyCmp key_cmp, pBHashDataCmp data_cmp, pBHashChkData chk_data)
{
	m_get_bucket_id = get_bucket_id;
	m_key_cmp = key_cmp;
	m_data_cmp = data_cmp;
	m_chk_data = chk_data;

	memset(&m_rBHash, 0, sizeof(recBHash));

    m_rBHash.bucket_size_ = bucket_size;  //HASH桶的大小
	m_rBHash.data_size_ = data_size;     //数据长度

	/* 分配桶空间 */
	m_rBHash.bucket_ = (recBHashBucket *)malloc(sizeof(recBHashBucket) * bucket_size); /* 桶 */
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

	m_rBHash.node_total_ = hash->m_rBHash.node_total_; /* 节点总数 */
    m_rBHash.bucket_size_ = hash->m_rBHash.bucket_size_;  //HASH桶的大小
	m_rBHash.data_size_ = hash->m_rBHash.data_size_;     //数据长度

	/* 分配桶空间 */
	m_rBHash.bucket_ = (recBHashBucket *)malloc(sizeof(recBHashBucket) * m_rBHash.bucket_size_); /* 桶 */
	if (m_rBHash.bucket_ == NULL)
		return;
	/* 复制BUCKET数据 */
	memcpy(m_rBHash.bucket_, hash->m_rBHash.bucket_, sizeof(recBHashBucket) * m_rBHash.bucket_size_);

	/* 分配数据空间 */
	m_rBHash.data_ = (char *)malloc(m_rBHash.node_total_ * m_rBHash.data_size_);
	if (m_rBHash.data_ == NULL)
		return;
	/* 复制所有数据 */
	memcpy(m_rBHash.data_, hash->m_rBHash.data_, m_rBHash.node_total_ * m_rBHash.data_size_);
}

CBHash::~CBHash()
{
	/* 释放资源 */
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
 * Desc: 新增准备,计算每个桶冲突数目
 * In: 
 *     void *   key        键值
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CBHash::pre_add(const void * key)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recBHashBucket * bucket = m_rBHash.bucket_ + id;

	if (bucket->num >= 0xFF)
		return -1;

	bucket->num++; /* 同一桶中冲突数+1 */

	m_rBHash.node_total_++; /* 总节点数+1 */

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
int CBHash::pre_finish()
{
	/* 分配数据空间 */
	m_rBHash.data_ = (char *)malloc(m_rBHash.node_total_ * get_data_size());
	if (m_rBHash.data_ == NULL)
		return -1;

	size_t offset = 0;
	for (int i=0; i<get_bucket_size(); i++)
	{
		recBHashBucket * bucket = m_rBHash.bucket_ + i;

		if (bucket->num)
		{
			/* 桶对应起始地址 */
			bucket->addr = offset;

			/* 下一个桶起始地址 */
			offset += bucket->num * get_data_size();

			/* 清缓冲 */
			size_t addr = get_node_addr() + bucket->addr;
			memset((char *)addr, 0, bucket->num * get_data_size());
		}
	}

	return 0;
}

/*
 * Function: add
 * Desc: 新增数据
 * In: 
 *     const void *   key        键值
 *     const void *   data        数据
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
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
			/* 存放数据 */
			memcpy(ptr, data, get_data_size());
			return 0;
		}

		ptr += get_data_size();
	}

	return -1;
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
int CBHash::add_finish()
{
	/* 数据地址 */
	size_t addr = get_node_addr();
	for (int i=0; i<get_bucket_size(); i++)
	{
		recBHashBucket * bucket = m_rBHash.bucket_ + i;

		if (bucket->num > 0)
		{
			/* 排序 */
			qsort((void *)(bucket->addr + addr), bucket->num, get_data_size(), CBHash::m_data_cmp);
		}
	}

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
int CBHash::get(const void * key, void * data)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recBHashBucket * bucket = m_rBHash.bucket_ + id;

	if (bucket->num <= 0)
		/* 桶中不存在节点 */
		return -1;

	char * ptr = (char *)(bucket->addr + get_node_addr());

	/* 做二分查找 */
	int low = 0;
	int high = bucket->num - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * get_data_size();

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* 键值相同 */
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
