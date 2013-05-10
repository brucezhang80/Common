/*
 * @File: merge.h
 * @Desc: head file of Multi-Way Merge
 * @Author: stevetang
 * @History:
 *      2008-12-05   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _MERGE_H
#define _MERGE_H

#include "CFileSeqRead.h"
#include "CFileSeqWrite.h"

#pragma pack(1)

class CMulMerge
{
	typedef struct _tagMergeInInfo {
		CFileSeqRead * in;  /* ˳����ļ����� */
		unsigned char valid;  /* ��Ч���� */
		int len; /* ���ݳ��� */
		char * data; /* ���� */
	} recMergeInInfo;

	typedef struct _tagMergeOutInfo {
		CFileSeqWrite * out; /* ˳��д�ļ����� */
		unsigned char isnew; /* �Ƿ����ռ� */
		unsigned char valid;  /* ��Ч���� */
		int len; /* ���ݳ��� */
		char * data; /* ���� */
	} recMergeOutInfo;

	int m_num; /* �鲢�ļ���Ŀ */
	int m_buflen; /* �������ݴ�С */
	int m_datalen; /* ������ݳ��� */
	int m_cur; /* ��ǰ�����ļ���Ŀ */

	recMergeInInfo ** m_InInfo; /* �������ڹ鲢������ */
	recMergeOutInfo * m_OutInfo; /* �������ڹ鲢������� */

public:
	CMulMerge(int num, int buflen, int datalen);
	virtual ~CMulMerge();

protected:
    /*
     * Function: Compare
     * Desc: �������ݱȽ�
     * In: 
     *     int    a
     *     int    b
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *     >0                   �ɹ�
     */
	virtual int Compare(int a, int b) = 0;

	/*
     * Function: SetOut
     * Desc: �����������
     * In: 
     *     int   min
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *     >0                   �ɹ�
     */
	virtual int SetOut(int min) = 0;

	/*
     * Function: GetInData
     * Desc: ��ȡ��������
     * In: 
     *     int   n              ��Ӧ��������
     * Out: 
     *     none
     * Return code: 
     *     NULL                 ����
     *     ��NULL               �ɹ�
     */
	const char * GetInData(int n);

	/*
     * Function: GetOutData
     * Desc: ��ȡ�������
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     NULL                 ����
     *     ��NULL               �ɹ�
     */
	char * GetOutData();

	/*
     * Function: SetOutData
     * Desc: �����������
     * In: 
     *     const char *   data  ��������
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int SetOutData(const char * data);

private:
    /*
     * Function: GetMin
     * Desc: ��ȡ��С����������
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *     >0                   �ɹ�
     */
	virtual int GetMin();

public:
    /*
     * Function: AddInFile
     * Desc: ���������ļ�
     * In: 
     *     const char *    fname   �ļ���
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int AddInFile(const char * fname, int len);

    /*
     * Function: SetOutFile
     * Desc: ��������ļ���
     * In: 
     *     const char *    fname   �ļ���
	 *     unsigned char   isnew   �Ƿ�����µĿռ䣬���������ṹ��ͬʱ��
	 *     int             len     ����ṹ��С
	 *     int             size    ����ļ������С
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int SetOutFile(const char * fname, unsigned char isnew, int len);

	/*
     * Function: Merge
     * Desc: ��·�鲢
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Merge();

	/*
     * Function: Close
     * Desc: ��ʼ�������ļ�������ļ���Ŀ
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Close();
};

#pragma pack()

#endif
