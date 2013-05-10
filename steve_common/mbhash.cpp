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

pMBHashGetBucketId CMBHash::m_get_bucket_id = NULL; /* 取桶ID */
pMBHashKeyCmp CMBHash::m_key_cmp = NULL; /* 键值比较 */
pMBHashDataCmp CMBHash::m_data_cmp = NULL; /* 数据比较 */
pMBHashChkData CMBHash::m_chk_data = NULL; /* 校验数据有效性 */

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
 * Desc: 打开MAP
 * In: 
 *     int                   shm_size   共享内存大小
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CMBHash::open(int shm_size /* = 0 */, unsigned char init /* = 0 */)
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

		/* 构造hash结构 */
		m_hash = (recMBHash *)m_shm.get_segment_ptr();

		/* 清零 */
		memset((char *)m_hash, 0, shm_size);

		/* 设置HASH信息 */
		m_hash->node_total_ = 0;
		m_hash->bucket_size_ = m_bucket_size;
		m_hash->data_size_ = m_data_size;
	}
	else
	{
		/* 共享内存存在，且大小相同，跟hash结构绑定 */
		m_hash = (recMBHash *)m_shm.get_segment_ptr();

		if (shm_size && init)
		{
			/* 清零 */
			memset((char *)m_hash, 0, shm_size);

			/* 设置HASH信息 */
			m_hash->node_total_ = 0;
			m_hash->bucket_size_ = m_bucket_size;
			m_hash->data_size_ = m_data_size;
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
int CMBHash::close()
{
	m_shm.detach();

	return 0;
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
int CMBHash::pre_add(const void * key)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recMBHashBucket * bucket = m_hash->bucket_ + id;

	if (bucket->num >= 0xFF)
		return -1;

	bucket->num++; /* 同一桶中冲突数+1 */

	m_hash->node_total_++; /* 总节点数+1 */

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
int CMBHash::pre_finish()
{
	size_t offset = 0; /* 记载偏移地址 */

	for (int i=0; i<m_bucket_size; i++)
	{
		recMBHashBucket * bucket = m_hash->bucket_ + i;

		if (bucket->num)
		{
			/* 桶对应起始地址 */
			bucket->addr = offset;

			/* 下一个桶起始地址 */
			offset += bucket->num * m_data_size;

			/* 清缓冲 */
			size_t addr = get_node_addr() + bucket->addr;
			memset((char *)addr, 0, bucket->num * m_data_size);
		}
	}

	return 0;
}

/*
 * Function: add
 * Desc: 新增数据
 * In: 
 *     void *   data        数据
 *     int      data_size   数据长度
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CMBHash::add(const void * key, const void * data)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recMBHashBucket * bucket = m_hash->bucket_ + id;

	/* 数据地址 */
	char * ptr = (char *)(bucket->addr + get_node_addr());

	for (int i=0; i<bucket->num; i++)
	{
		if (!m_chk_data(ptr))
		{
			/* 存放数据 */
			memcpy(ptr, data, m_data_size);
			return 0;
		}

		ptr += m_data_size;
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
int CMBHash::add_finish()
{
	/* 数据地址 */
	size_t addr = get_node_addr();
	for (int i=0; i<m_bucket_size; i++)
	{
		recMBHashBucket * bucket = m_hash->bucket_ + i;

		if (bucket->num > 0)
		{
			/* 排序 */
			qsort((void *)(bucket->addr + addr), bucket->num, m_data_size, CMBHash::m_data_cmp);
		}
	}

	return 0;
}

/*
 * Function: get
 * Desc: 获取数据
 * In: 
 *     void *   data        键值
 * Out: 
 *     void *   data        数据
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CMBHash::get(const void * key, void * data)
{
	int id = m_get_bucket_id(key) % get_bucket_size();
	recMBHashBucket * bucket = m_hash->bucket_ + id;

	/* 数据地址 */
	char * ptr = (char *)(bucket->addr + get_node_addr());

	/* 做二分查找 */
	int low = 0;
	int high = bucket->num - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * m_data_size;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* 键值相同 */
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
