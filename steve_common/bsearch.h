/*
 * @File: bsearch.h
 * @Desc: head file of Binary Search
 * @Author: stevetang
 * @History:
 *      2009-01-09   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _BSEARCH_H
#define _BSEARCH_H

#pragma pack(1)

typedef struct _tagBSearch {
	unsigned int node_total_; /* �ڵ����� */
	unsigned int data_size_; /* ���ݳ��� */
	char * data_; /* ���� */
} recBSearch;

#pragma pack()

//��ֵ�Ƚ� 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pBSearchKeyCmp)(const void *, const void *);

//��ֵ�Ƚ� 0-"1=2" >0- "1>2" <0-"1<2"
typedef int (* pBSearchDataCmp)(const void *, const void *);

class CBSearch
{
	unsigned int m_cur; /* ��ǰ�ڵ��� */
	recBSearch m_rBSearch; /* ������Ϣ */

	pBSearchKeyCmp m_key_cmp; /* ��ֵ�Ƚ� */
	static pBSearchDataCmp m_data_cmp; /* ���ݱȽ� */

public:
	CBSearch(int data_size, pBSearchKeyCmp key_cmp, pBSearchDataCmp data_cmp);
	~CBSearch();

public:
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

	int get_total_node() { return m_rBSearch.node_total_; };
	int get_data_size() { return m_rBSearch.data_size_; };
	size_t get_data_addr() { return (size_t)m_rBSearch.data_; };
};

#endif
