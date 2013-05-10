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
		CFileSeqRead * in;  /* 顺序读文件对象 */
		unsigned char valid;  /* 有效数据 */
		int len; /* 数据长度 */
		char * data; /* 数据 */
	} recMergeInInfo;

	typedef struct _tagMergeOutInfo {
		CFileSeqWrite * out; /* 顺序写文件对象 */
		unsigned char isnew; /* 是否分配空间 */
		unsigned char valid;  /* 有效数据 */
		int len; /* 数据长度 */
		char * data; /* 数据 */
	} recMergeOutInfo;

	int m_num; /* 归并文件数目 */
	int m_buflen; /* 缓冲数据大小 */
	int m_datalen; /* 最大数据长度 */
	int m_cur; /* 当前输入文件数目 */

	recMergeInInfo ** m_InInfo; /* 保存用于归并的数据 */
	recMergeOutInfo * m_OutInfo; /* 保存用于归并后的数据 */

public:
	CMulMerge(int num, int buflen, int datalen);
	virtual ~CMulMerge();

protected:
    /*
     * Function: Compare
     * Desc: 进行数据比较
     * In: 
     *     int    a
     *     int    b
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *     >0                   成功
     */
	virtual int Compare(int a, int b) = 0;

	/*
     * Function: SetOut
     * Desc: 设置输出数据
     * In: 
     *     int   min
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *     >0                   成功
     */
	virtual int SetOut(int min) = 0;

	/*
     * Function: GetInData
     * Desc: 获取输入数据
     * In: 
     *     int   n              对应输入数据
     * Out: 
     *     none
     * Return code: 
     *     NULL                 出错
     *     非NULL               成功
     */
	const char * GetInData(int n);

	/*
     * Function: GetOutData
     * Desc: 获取输出数据
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     NULL                 出错
     *     非NULL               成功
     */
	char * GetOutData();

	/*
     * Function: SetOutData
     * Desc: 设置输出数据
     * In: 
     *     const char *   data  输入数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int SetOutData(const char * data);

private:
    /*
     * Function: GetMin
     * Desc: 获取最小的输入数据
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *     >0                   成功
     */
	virtual int GetMin();

public:
    /*
     * Function: AddInFile
     * Desc: 增加输入文件
     * In: 
     *     const char *    fname   文件名
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int AddInFile(const char * fname, int len);

    /*
     * Function: SetOutFile
     * Desc: 设置输出文件名
     * In: 
     *     const char *    fname   文件名
	 *     unsigned char   isnew   是否分配新的空间，输出跟输入结构不同时用
	 *     int             len     输出结构大小
	 *     int             size    输出文件缓冲大小
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int SetOutFile(const char * fname, unsigned char isnew, int len);

	/*
     * Function: Merge
     * Desc: 多路归并
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Merge();

	/*
     * Function: Close
     * Desc: 初始化输入文件、输出文件数目
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Close();
};

#pragma pack()

#endif
