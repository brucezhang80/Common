/*
 * @File: mbhash.h
 * @Desc: head file of Binary-Hash
 * @Author: stevetang
 * @History:
 *      2008-12-04   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _MBHASH_H
#define _MBHASH_H

/* 固定数据，不加锁 */

#include "shm.h"

/* 键值保存在数据区里，不单独保存键值;支持同一桶中最多255个冲突 */

#pragma pack(1)

typedef struct _tagMBHashBucket {
	unsigned char num; /* 对应桶中数目 */
	size_t addr; /* 桶对应数据地址 */
} recMBHashBucket;

typedef struct _tagMBHash {
    int node_total_;   //节点总数
    int bucket_size_;  //HASH桶的大小
	int data_size_;     //数据长度
	recMBHashBucket bucket_[0]; /* 桶地址 */
} recMBHash;

#pragma pack()

//取桶ID
typedef int (* pMBHashGetBucketId)(const void *);

//键值比较 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pMBHashKeyCmp)(const void *, const void *);

//键值比较 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pMBHashDataCmp)(const void *, const void *);

//校验数据有效性 false-无效 true-有效
typedef bool (* pMBHashChkData)(const void *);

class CMBHash
{
	int m_shm_key; /* 共享内存键值 */
    CSVShm m_shm; /* 共享内存 */

    recMBHash * m_hash; /* 对应HASH结构 */

	int m_bucket_size; /* 桶数目 */
	int m_data_size; /* 数据长度 */

	static pMBHashGetBucketId m_get_bucket_id; /* 取桶ID */
	static pMBHashKeyCmp m_key_cmp; /* 键值比较 */
	static pMBHashDataCmp m_data_cmp; /* 数据比较 */
	static pMBHashChkData m_chk_data; /* 校验数据有效性 */

public:
	CMBHash(int shm_key, int bucket_size, int data_size, pMBHashGetBucketId get_bucket_id, 
			pMBHashKeyCmp key_cmp, pMBHashDataCmp data_cmp, pMBHashChkData chk_data);
	~CMBHash();

public:
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
     * Desc: 新增准备,计算每个桶冲突数目
     * In: 
     *     void *   key        键值
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int pre_add(const void * key);

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
	 *     void *   data        数据
	 *     int      data_size   数据长度
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int add(const void * key, const void * data);

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
     *     void *   data        键值
     * Out: 
     *     void *   data        数据
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int get(const void * key, void * data);

	/* 取HASH信息 */
	int get_shm_key() { return m_shm_key; };
	int get_node_total() { return m_hash->node_total_; };
	int get_bucket_size() { return m_hash->bucket_size_; };
	int get_data_size() { return m_hash->data_size_; };
	size_t get_bucket_addr() { return (size_t)m_hash->bucket_; };
	size_t get_node_addr() { return (get_bucket_addr() + (unsigned int)get_bucket_size() * sizeof(recMBHashBucket)); };

	int get_need_total_size(int node_total) { 
		int need = sizeof(recMBHash) + sizeof(recMBHashBucket) * m_bucket_size + node_total * m_data_size; 
		return need; 
	};

	recMBHash * get_hash() const { return m_hash; };
};

#endif
