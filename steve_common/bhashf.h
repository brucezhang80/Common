/*
 * @File: bhashf.h
 * @Desc: head file of Binary-Hash File
 * @Author: stevetang
 * @History:
 *      2009-04-23   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _BHASHF_H
#define _BHASHF_H

#include "bfile.h"
#include "CFileSeqWrite.h"

typedef int (* pBHashFGetBucketID)(const void *);
typedef int (* pBHashFCmpKey)(const void *, const void *);
typedef int (* pBHashFEnumIdx)(unsigned int, const void *);
typedef int (* pBHashFEnumData)(unsigned int, void *, unsigned int, unsigned char *);
typedef int (* pBHashFEnumDataEx)(unsigned int, void *, unsigned int, unsigned char *);

#pragma pack(1)

typedef struct _tagBHashFIdxInfo {
	unsigned char f_id; /* 索引文件编号 */
	unsigned char num; /* 桶中数据数目 */
} recBHashFIdxInfo;

typedef struct _tagBHashFInfo {
	char name_[128]; /* 文件名 */
	unsigned int isize_; /* 桶数目 */
	unsigned int ilen_; /* 初始索引文件单元长度 */
	unsigned char klen_; /* 键值长度 */
	unsigned int dlen_; /* 数据长度 */
} recBHashFInfo;

#pragma pack()

#define BHASHF_SUFFIX_INFO      ".info"
#define BHASHF_SUFFIX_IDX		".idx"
#define BHASHF_SUFFIX_DATA		".dat"

class CBaseBHashF
{
	char m_path[128]; /* 路径名 */
	char m_fname[128]; /* 文件名前缀 */

	enuBFileType m_openType; /* 打开类型 */

public:
	CBaseBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024, int max_fcnt = 10, int mul = 2, int max_size = 0);
	virtual ~CBaseBHashF();

protected:
	pBHashFGetBucketID m_get_bucket_id; /* 获取桶编号 */
	pBHashFCmpKey m_key_cmp; /* 键值比较 */

	recBHashFInfo m_bHashF; /* 文件信息 */

	CMBFile m_idxInfo; /* 索引信息 */
	CBFile m_dataInfo; /* 基本数据文件 */

	int m_max_fcnt; /* 最大数据扩展文件数 */
	int m_mul; /* 递增倍数 */
	CBFileEx ** m_dataInfoEx; /* 扩展数据文件信息 */

	int m_buflen; /* 缓冲数据大小 */
	char * m_buf; /* 缓冲 */
	char * m_tmp_buf; /* 临时数据存储 */

private:
    /*
     * Function: IsExist
     * Desc: 检查数据是否存在
     * In: 
     *     const char *    path    文件名
     *     const char *    fname   文件名
     * Out: 
     *     none
     * Return code: 
     *      0                   不存在
     *      1                   存在
     */
	bool IsExist(const char * path, const char * fname);

	/*
	 * Function: WriteData
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	virtual int WriteData(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: WriteDataEx
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	virtual int WriteDataEx(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: ReadData
	 * Desc: 读数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	virtual int ReadData(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: ReadDataEx
	 * Desc: 读数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	virtual int ReadDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: DelData
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	virtual int DelData(int site, recBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

	/*
	 * Function: DelDataEx
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	virtual int DelDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

protected:
	/*
	 * Function: GetData
	 * Desc: 获取数据
	 * In: 
	 *     const void *   key         键值
	 * Out: 
	 *     void *         data        数据
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int GetData(const void * buf, int num, int ilen, const void * key, char * &data);

	/*
	 * Function: WriteDataEx
	 * Desc: 写数据
	 * In: 
	 *     int             site     桶
     *     const void *    data     数据
	 *     int             len      数据长度
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *     >0                   文件编号
	 */
	int WriteDataEx(int site, const void * data, int len);

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
	int Open(const char * path, const char * fname, int bucket = 7, int ilen = 1024, int klen = 0, int dlen = 0, enuBFileType openType = E_READ);

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
     * Function: SetChunk
     * Desc: 设置区间段
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int SetChunk(int start, int end);

    /*
     * Function: Write
     * Desc: 写数据
     * In: 
     *     const void *    key      键值
     *     const void *    data     数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Write(const void * key, const void * data, int datalen, unsigned char force = 1);

    /*
     * Function: Delete
     * Desc: 删除
     * In: 
     *     const void *    key      键值
     *     const void *    data     数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Delete(const void * key, void * data = NULL, int * datalen = NULL);

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
	int Read(const void * key, void * data, int& datalen);

    /*
     * Function: Flush
     * Desc: 临时缓冲数据写回实际数据区
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Flush();

    /*
     * Function: Attach
     * Desc: 使用外来的缓冲
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Attach(int maxsize, char * data);

    /*
     * Function: Detach
     * Desc: 临时缓冲数据写回实际数据区
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Detach();

	/*
     * Function: EnumIdx
     * Desc: 枚举索引数据并进行处理
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int EnumIdx(const char * path, const char * fname, pBHashFEnumIdx func);

    /*
     * Function: EnumData
     * Desc: 枚举数据并进行处理
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int EnumData(const char * path, const char * fname, pBHashFEnumData func);

    /*
     * Function: EnumDataEx
     * Desc: 枚举扩展数据文件并进行处理
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int EnumDataEx(const char * path, const char * fname, pBHashFEnumDataEx func);
};

class CBHashF : public CBaseBHashF
{
public:
	CBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024, int max_fcnt = 10, int mul = 2, int max_size = 0);
	~CBHashF();

public:
    /*
     * Function: Write
     * Desc: 写数据
     * In: 
     *     const void *    key      键值
     *     const void *    data     数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Write(const void * key, const void * data, unsigned char force = 1);

    /*
     * Function: Delete
     * Desc: 删除
     * In: 
     *     const void *    key      键值
     *     const void *    data     数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Delete(const void * key, void * data = NULL);

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
	int Read(const void * key, void * data);
};

/* 变长数据；块内数据结构为：key+len+data;支持最大长度为65535 */
class CBHashFEx : public CBaseBHashF
{
public:
	CBHashFEx(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024, int max_fcnt = 10, int mul = 2, int max_size = 0);
	~CBHashFEx();

private:
	/*
	 * Function: PreSort
	 * Desc: 生成排序数据
	 * In: 
     *     const char *    src      源数据串
	 * Out: 
     *     char *          dst      数据
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int PreSort(int num, const char * src, char * dst);

	/*
	 * Function: WriteData
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int WriteData(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: WriteDataEx
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int WriteDataEx(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: ReadData
	 * Desc: 读数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int ReadData(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: ReadDataEx
	 * Desc: 读数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int ReadDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: DelData
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int DelData(int site, recBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

	/*
	 * Function: DelDataEx
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int DelDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

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
	int Open(const char * path, const char * fname, int bucket = 7, int ilen = 1024, int klen = 0, enuBFileType openType = E_READ);
};

#pragma pack(1)

typedef struct _tagMinBHashFIdxInfo {
	unsigned char num; /* 桶中数据数目 */
} recMinBHashFIdxInfo;

#pragma pack()

#define MINBHASHF_SUFFIX_INFO      ".info"
#define MINBHASHF_SUFFIX_IDX		".idx"
#define MINBHASHF_SUFFIX_DATA		".dat"

class CBaseMinBHashF
{
	char m_path[128]; /* 路径名 */
	char m_fname[128]; /* 文件名前缀 */

	enuBFileType m_openType; /* 打开类型 */

public:
	CBaseMinBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024);
	virtual ~CBaseMinBHashF();

protected:
	pBHashFGetBucketID m_get_bucket_id; /* 获取桶编号 */
	pBHashFCmpKey m_key_cmp; /* 键值比较 */

	recBHashFInfo m_bHashF; /* 文件信息 */

	CMBFile m_idxInfo; /* 索引信息 */
	CBFileEx m_dataInfo; /* 扩展数据文件信息 */

	int m_buflen; /* 缓冲数据大小 */
	char * m_buf; /* 缓冲 */
	char * m_tmp_buf; /* 临时数据存储 */

private:
    /*
     * Function: IsExist
     * Desc: 检查数据是否存在
     * In: 
     *     const char *    path    文件名
     *     const char *    fname   文件名
     * Out: 
     *     none
     * Return code: 
     *      0                   不存在
     *      1                   存在
     */
	bool IsExist(const char * path, const char * fname);

	/*
	 * Function: WriteDataEx
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	virtual int WriteData(int site, recMinBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: ReadDataEx
	 * Desc: 读数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	virtual int ReadData(int site, recMinBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: DelData
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	virtual int DelData(int site, recMinBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

protected:
	/*
	 * Function: GetData
	 * Desc: 获取数据
	 * In: 
	 *     const void *   key         键值
	 * Out: 
	 *     void *         data        数据
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int GetData(const void * buf, int num, int ilen, const void * key, char * &data);

	/*
	 * Function: WriteDataEx
	 * Desc: 写数据
	 * In: 
	 *     int             site     桶
     *     const void *    data     数据
	 *     int             len      数据长度
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *     >0                   文件编号
	 */
	int WriteData(int site, const void * data, int len);

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
	int Open(const char * path, const char * fname, int bucket = 7, int ilen = 1024, int klen = 0, int dlen = 0, enuBFileType openType = E_READ);

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
     *     const void *    data     数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Write(const void * key, const void * data, int datalen, unsigned char force = 1);

    /*
     * Function: Delete
     * Desc: 删除
     * In: 
     *     const void *    key      键值
     *     const void *    data     数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Delete(const void * key, void * data = NULL, int * datalen = NULL);

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
	int Read(const void * key, void * data, int& datalen);

	/*
     * Function: EnumIdx
     * Desc: 枚举索引数据并进行处理
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int EnumIdx(const char * path, const char * fname, pBHashFEnumIdx func);

    /*
     * Function: EnumDataEx
     * Desc: 枚举扩展数据文件并进行处理
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int EnumData(const char * path, const char * fname, pBHashFEnumDataEx func);
};

class CMinBHashF : public CBaseMinBHashF
{
public:
	CMinBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024);
	~CMinBHashF();

public:
    /*
     * Function: Write
     * Desc: 写数据
     * In: 
     *     const void *    key      键值
     *     const void *    data     数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Write(const void * key, const void * data, unsigned char force = 1);

    /*
     * Function: Delete
     * Desc: 删除
     * In: 
     *     const void *    key      键值
     *     const void *    data     数据
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Delete(const void * key, void * data = NULL);

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
	int Read(const void * key, void * data);
};

/* 变长数据；块内数据结构为：key+len+data;支持最大长度为65535 */
class CMinBHashFEx : public CBaseMinBHashF
{
public:
	CMinBHashFEx(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024);
	~CMinBHashFEx();

private:
	/*
	 * Function: PreSort
	 * Desc: 生成排序数据
	 * In: 
     *     const char *    src      源数据串
	 * Out: 
     *     char *          dst      数据
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int PreSort(int num, const char * src, char * dst);

	/*
	 * Function: WriteData
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int WriteData(int site, recMinBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: ReadData
	 * Desc: 读数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int ReadData(int site, recMinBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: DelData
	 * Desc: 写数据
	 * In: 
     *     const void *    key      键值
     *     const void *    data     数据
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   出错
	 *      0                   成功
	 */
	int DelData(int site, recMinBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

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
	int Open(const char * path, const char * fname, int bucket = 7, int ilen = 1024, int klen = 0, enuBFileType openType = E_READ);
};

#endif
