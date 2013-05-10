/*
 * @File: bhash.h
 * @Desc: head file of Binary-Hash
 * @Author: stevetang
 * @History:
 *      2008-12-24   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _BHASH_H
#define _BHASH_H

/* 键值保存在数据区里，不单独保存键值;支持同一桶中最多255个冲突 */

#pragma pack(1)

typedef struct _tagBHashBucket {
	unsigned char num; /* 对应桶中数目 */
	size_t addr; /* 桶对应数据地址 */
} recBHashBucket;

typedef struct _tagBHash {
    int node_total_;   //节点总数
    int bucket_size_;  //HASH桶的大小
	int data_size_;     //数据长度

	recBHashBucket * bucket_; /* 桶 */
	char * data_; /* 数据 */
} recBHash;

#pragma pack()

//取桶ID
typedef int (* pBHashGetBucketId)(const void *);

//键值比较 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pBHashKeyCmp)(const void *, const void *);

//键值比较 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pBHashDataCmp)(const void *, const void *);

//校验数据有效性 false-无效 true-有效
typedef bool (* pBHashChkData)(const void *);

class CBHash
{
	recBHash m_rBHash; /* HASH表信息 */

	static pBHashGetBucketId m_get_bucket_id; /* 取桶编号 */
	static pBHashKeyCmp m_key_cmp; /* 键值比较 */
	static pBHashDataCmp m_data_cmp; /* 数据值比较 */
	static pBHashChkData m_chk_data; /* 校验数据有效性 */

public:
	CBHash(int bucket_size, int data_size, pBHashGetBucketId get_bucket_id, pBHashKeyCmp key_cmp, pBHashDataCmp data_cmp, pBHashChkData chk_data);
	CBHash(const CBHash * hash);
	~CBHash();

public:
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
     *     const void *   key        键值
	 *     const void *   data        数据
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
     *     const void *   key         键值
     * Out: 
     *     void *         data        数据
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int get(const void * key, void * data);

	/* 取HASH信息 */
	int get_node_total() { return m_rBHash.node_total_; };
	int get_bucket_size() { return m_rBHash.bucket_size_; };
	int get_data_size() { return m_rBHash.data_size_; };
	size_t get_node_addr() { return (size_t)m_rBHash.data_; };
	size_t get_bucket_addr() { return (size_t)m_rBHash.bucket_; };
};

#endif
