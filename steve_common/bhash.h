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

/* ��ֵ������������������������ֵ;֧��ͬһͰ�����255����ͻ */

#pragma pack(1)

typedef struct _tagBHashBucket {
	unsigned char num; /* ��ӦͰ����Ŀ */
	size_t addr; /* Ͱ��Ӧ���ݵ�ַ */
} recBHashBucket;

typedef struct _tagBHash {
    int node_total_;   //�ڵ�����
    int bucket_size_;  //HASHͰ�Ĵ�С
	int data_size_;     //���ݳ���

	recBHashBucket * bucket_; /* Ͱ */
	char * data_; /* ���� */
} recBHash;

#pragma pack()

//ȡͰID
typedef int (* pBHashGetBucketId)(const void *);

//��ֵ�Ƚ� 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pBHashKeyCmp)(const void *, const void *);

//��ֵ�Ƚ� 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pBHashDataCmp)(const void *, const void *);

//У��������Ч�� false-��Ч true-��Ч
typedef bool (* pBHashChkData)(const void *);

class CBHash
{
	recBHash m_rBHash; /* HASH����Ϣ */

	static pBHashGetBucketId m_get_bucket_id; /* ȡͰ��� */
	static pBHashKeyCmp m_key_cmp; /* ��ֵ�Ƚ� */
	static pBHashDataCmp m_data_cmp; /* ����ֵ�Ƚ� */
	static pBHashChkData m_chk_data; /* У��������Ч�� */

public:
	CBHash(int bucket_size, int data_size, pBHashGetBucketId get_bucket_id, pBHashKeyCmp key_cmp, pBHashDataCmp data_cmp, pBHashChkData chk_data);
	CBHash(const CBHash * hash);
	~CBHash();

public:
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
	int pre_add(const void * key);

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
	int pre_finish();

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
	int add(const void * key, const void * data);

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
	int add_finish();

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
	int get(const void * key, void * data);

	/* ȡHASH��Ϣ */
	int get_node_total() { return m_rBHash.node_total_; };
	int get_bucket_size() { return m_rBHash.bucket_size_; };
	int get_data_size() { return m_rBHash.data_size_; };
	size_t get_node_addr() { return (size_t)m_rBHash.data_; };
	size_t get_bucket_addr() { return (size_t)m_rBHash.bucket_; };
};

#endif
