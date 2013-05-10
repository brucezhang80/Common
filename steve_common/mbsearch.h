/*
 * @File: mbsearch.h
 * @Desc: head file of Binary Search in Share Memory
 * @Author: stevetang
 * @History:
 *      2009-01-09   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _MBSEARCH_H
#define _MBSEARCH_H

#include "shm.h"

#pragma pack(1)

typedef struct _tagMBSearch {
	unsigned int node_total_; /* �ڵ����� */
	unsigned int data_size_; /* ���ݳ��� */
	char data_[0]; /* ���� */
} recMBSearch;

#pragma pack()

//��ֵ�Ƚ� 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pMBSearchKeyCmp)(const void *, const void *);

//��ֵ�Ƚ� 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pMBSearchDataCmp)(const void *, const void *);

class CMBSearch
{
	int m_shm_key; /* �����ڴ��ֵ */
    CSVShm m_shm; /* �����ڴ� */

	int m_data_size; /* ���ݳ��� */

	unsigned int m_cur; /* ��ǰ�ڵ��� */
	recMBSearch * m_rMBSearch; /* ������Ϣ */

	static pMBSearchKeyCmp m_key_cmp; /* ��ֵ�Ƚ� */
	static pMBSearchDataCmp m_data_cmp; /* ���ݱȽ� */

public:
	CMBSearch(int shm_key, int data_size, pMBSearchKeyCmp key_cmp, pMBSearchDataCmp data_cmp);
	~CMBSearch();

public:
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
     * Desc: ����ڵ���Ŀ
     * In: 
     *     void *   data       ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int pre_add(const void * data);

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
	 *     const void *   data        ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int add(const void * data);

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

	int get_shm_key() { return m_shm_key; };
	int get_total_node() { return m_rMBSearch->node_total_; };
	int get_data_size() { return m_rMBSearch->data_size_; };
	size_t get_data_addr() { return (size_t)m_rMBSearch->data_; };

	recMBSearch * get_search() const { return m_rMBSearch; };
};

#endif
