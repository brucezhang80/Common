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

/* �̶����ݣ������� */

#include "shm.h"

/* ��ֵ������������������������ֵ;֧��ͬһͰ�����255����ͻ */

#pragma pack(1)

typedef struct _tagMBHashBucket {
	unsigned char num; /* ��ӦͰ����Ŀ */
	size_t addr; /* Ͱ��Ӧ���ݵ�ַ */
} recMBHashBucket;

typedef struct _tagMBHash {
    int node_total_;   //�ڵ�����
    int bucket_size_;  //HASHͰ�Ĵ�С
	int data_size_;     //���ݳ���
	recMBHashBucket bucket_[0]; /* Ͱ��ַ */
} recMBHash;

#pragma pack()

//ȡͰID
typedef int (* pMBHashGetBucketId)(const void *);

//��ֵ�Ƚ� 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pMBHashKeyCmp)(const void *, const void *);

//��ֵ�Ƚ� 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pMBHashDataCmp)(const void *, const void *);

//У��������Ч�� false-��Ч true-��Ч
typedef bool (* pMBHashChkData)(const void *);

class CMBHash
{
	int m_shm_key; /* �����ڴ��ֵ */
    CSVShm m_shm; /* �����ڴ� */

    recMBHash * m_hash; /* ��ӦHASH�ṹ */

	int m_bucket_size; /* Ͱ��Ŀ */
	int m_data_size; /* ���ݳ��� */

	static pMBHashGetBucketId m_get_bucket_id; /* ȡͰID */
	static pMBHashKeyCmp m_key_cmp; /* ��ֵ�Ƚ� */
	static pMBHashDataCmp m_data_cmp; /* ���ݱȽ� */
	static pMBHashChkData m_chk_data; /* У��������Ч�� */

public:
	CMBHash(int shm_key, int bucket_size, int data_size, pMBHashGetBucketId get_bucket_id, 
			pMBHashKeyCmp key_cmp, pMBHashDataCmp data_cmp, pMBHashChkData chk_data);
	~CMBHash();

public:
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
	int open(int shm_size = 0, unsigned char init = 0);

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
	int close();

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
	 *     void *   data        ����
	 *     int      data_size   ���ݳ���
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
     *     void *   data        ��ֵ
     * Out: 
     *     void *   data        ����
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int get(const void * key, void * data);

	/* ȡHASH��Ϣ */
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
