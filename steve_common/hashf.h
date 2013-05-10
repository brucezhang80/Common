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

/* HASH文件数据节点 */
typedef struct _tagHashFNode {
	unsigned long long next;
	unsigned int key_len;
	char * key;
	unsigned int data_len;
	char * data;
} recHashFNode;

typedef struct _tagHashF {
	char name[128]; /* 文件名 */
	int b_size; /* 桶数目 */
	unsigned long long addr; /* 数据地址 */
	unsigned long long bucket[0]; /* 桶数据 */
} recHashF;

#pragma pack()

class CHashF
{
	pHashFGetBucketID m_get_bucket_id; /* 获取桶编号 */
	pHashFCmpKey m_cmp_key; /* 键值比较 */

public:
	CHashF(pHashFGetBucketID get_bucket_id, pHashFCmpKey cmp_key, int buflen = 1024 * 1024);
	~CHashF();

public:
	typedef enum {
		E_READ = 0,
		E_WRITE
	} enuHashFType;

private:
	int m_fd; /* 文件句柄 */

	recHashF m_rHashF; /* 文件信息 */

	/* 用于枚举的变量 */
	char * m_buf; /* 临时缓冲 */
	int m_buflen; /* 临时缓冲大小 */

	recHashFNode * m_hashfNode; /* HASH数据节点 */

	/* 记载枚举的读写位置 */
	unsigned long long m_enum_r;  /* 读位置 */
	unsigned long long m_enum_w; /* 写位置 */

public:
    /*
     * Function: Open
     * Desc: 打开文件
     * In: 
     *     const char *    fname   文件名
	 *     int             bucket  桶
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Open(const char * path, int bucket = 7, enuHashFType openType = E_READ);

    /*
     * Function: Close
     * Desc: 关闭文件
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Close();

    /*
     * Function: Write
     * Desc: 写数据
     * In: 
     *     const void *    key      键值
	 *     int             key_len  键长度
     *     const void *    data     数据
	 *     int             data_len 数据长度
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Write(const void * key, int key_len, const void * data, int data_len);

    /*
     * Function: Read
     * Desc: 读数据
     * In: 
     *     const void *    key      键值
	 *     int             key_len  键长度
     * Out: 
     *     void *          data     数据
	 *     int&            data_len 数据长度
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Read(const void * key, int key_len, void * data, int& data_len);

    /*
     * Function: EnumBegin
     * Desc: 启动文件枚举
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int EnumBegin();

    /*
     * Function: EnumRead
     * Desc: 枚举数据
     * In: 
     *     none
     * Out: 
     *     recHashFNode * &rHashFNode  数据节点
	 *     int            &num         节点数目
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int EnumRead(recHashFNode * &rHashFNode, int &num);

    /*
     * Function: EnumWrite
     * Desc: 写回枚举数据,数据长度不能变
     * In: 
     *     recHashFNode * rHashFNode  数据节点
	 *     int            num         节点数目
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int EnumWrite(recHashFNode * rHashFNode, int num);

    /*
     * Function: EnumClose
     * Desc: 关闭文件枚举
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int EnumClose();
};

#endif
