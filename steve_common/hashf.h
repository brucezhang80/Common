/*
 * @File: hashf.h
 * @Desc: head file of Hash File
 * @Author: stevetang
 * @History:
 *      2008-12-15   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _HASHF_H
#define _HASHF_H

typedef int (* pHashFGetBucketID)(const void *);
typedef int (* pHashFCmpKey)(const void *, const void *);

#pragma pack(1)

/* HASH�ļ����ݽڵ� */
typedef struct _tagHashFNode {
	unsigned long long next;
	unsigned int key_len;
	char * key;
	unsigned int data_len;
	char * data;
} recHashFNode;

typedef struct _tagHashF {
	char name[128]; /* �ļ��� */
	int b_size; /* Ͱ��Ŀ */
	unsigned long long addr; /* ���ݵ�ַ */
	unsigned long long bucket[0]; /* Ͱ���� */
} recHashF;

#pragma pack()

class CHashF
{
	pHashFGetBucketID m_get_bucket_id; /* ��ȡͰ��� */
	pHashFCmpKey m_cmp_key; /* ��ֵ�Ƚ� */

public:
	CHashF(pHashFGetBucketID get_bucket_id, pHashFCmpKey cmp_key, int buflen = 1024 * 1024);
	~CHashF();

public:
	typedef enum {
		E_READ = 0,
		E_WRITE
	} enuHashFType;

private:
	int m_fd; /* �ļ���� */

	recHashF m_rHashF; /* �ļ���Ϣ */

	/* ����ö�ٵı��� */
	char * m_buf; /* ��ʱ���� */
	int m_buflen; /* ��ʱ�����С */

	recHashFNode * m_hashfNode; /* HASH���ݽڵ� */

	/* ����ö�ٵĶ�дλ�� */
	unsigned long long m_enum_r;  /* ��λ�� */
	unsigned long long m_enum_w; /* дλ�� */

public:
    /*
     * Function: Open
     * Desc: ���ļ�
     * In: 
     *     const char *    fname   �ļ���
	 *     int             bucket  Ͱ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Open(const char * path, int bucket = 7, enuHashFType openType = E_READ);

    /*
     * Function: Close
     * Desc: �ر��ļ�
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Close();

    /*
     * Function: Write
     * Desc: д����
     * In: 
     *     const void *    key      ��ֵ
	 *     int             key_len  ������
     *     const void *    data     ����
	 *     int             data_len ���ݳ���
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Write(const void * key, int key_len, const void * data, int data_len);

    /*
     * Function: Read
     * Desc: ������
     * In: 
     *     const void *    key      ��ֵ
	 *     int             key_len  ������
     * Out: 
     *     void *          data     ����
	 *     int&            data_len ���ݳ���
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Read(const void * key, int key_len, void * data, int& data_len);

    /*
     * Function: EnumBegin
     * Desc: �����ļ�ö��
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int EnumBegin();

    /*
     * Function: EnumRead
     * Desc: ö������
     * In: 
     *     none
     * Out: 
     *     recHashFNode * &rHashFNode  ���ݽڵ�
	 *     int            &num         �ڵ���Ŀ
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int EnumRead(recHashFNode * &rHashFNode, int &num);

    /*
     * Function: EnumWrite
     * Desc: д��ö������,���ݳ��Ȳ��ܱ�
     * In: 
     *     recHashFNode * rHashFNode  ���ݽڵ�
	 *     int            num         �ڵ���Ŀ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int EnumWrite(recHashFNode * rHashFNode, int num);

    /*
     * Function: EnumClose
     * Desc: �ر��ļ�ö��
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int EnumClose();
};

#endif
