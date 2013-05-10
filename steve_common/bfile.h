/*
 * @File: bfile.h
 * @Desc: head file of Bitmap File
 * @Author: stevetang
 * @History:
 *      2009-04-28   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _BFILE_H
#define _BFILE_H

#pragma pack(1)

typedef enum {
	E_READ = 0,
	E_WRITE
} enuBFileType;

typedef struct _tagMBFile {
	char name_[128]; /* 文件名 */
	unsigned int isize_; /* 节点数 */
	unsigned int ilen_; /* 节点单元长度 */
	unsigned char data_[0]; /* 数据区 */	
} recMBFile;

#pragma pack()

class CMBFile
{
	int m_fd; /* 文件句柄 */
	enuBFileType m_openType; /* 打开类型 */
	recMBFile * m_bFile; /* 相关文件信息 */

public:
	CMBFile();
	~CMBFile();

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
	int Open(const char * path, int isize, int ilen, enuBFileType openType = E_READ);

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
     * Function: Read
     * Desc: 读数据
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     NULL                出错
     *     非NULL              成功
     */
	void * Read(unsigned int site);
};

#pragma pack(1)

typedef struct _tagBFile {
	char name_[128]; /* 文件名 */
	unsigned int isize_; /* 节点数 */
	unsigned int ilen_; /* 节点单元长度 */
	unsigned long long addr_; /* 数据地址 */
	unsigned char binfo_[0]; /* 节点的bitmap信息 */
	/* 数据区 */
} recBFile;

#pragma pack()

#define BFILE_GET_VALID(info_, site, bit) \
	{ \
		bit = ((info_[site / 8] >> (site % 8)) & 0x01); \
	}

#define BFILE_SET_VALID(info_, site, bit) \
	{ \
		if (bit) \
			info_[site / 8] |= (0x01 << (site % 8)); \
		else \
			info_[site / 8] &= ~(0x01 << (site % 8)); \
	}

class CBFile
{
	int m_fd; /* 文件句柄 */
	enuBFileType m_openType; /* 打开类型 */
	recBFile * m_bFile; /* 相关文件信息 */

	int m_buflen; /* 缓冲数据大小 */
	char * m_buf; /* 临时数据缓冲 */

	/* 映射部分数据，用于大数据量处理 */
	unsigned int m_start; /* 映射的起始桶号 */
	unsigned int m_end; /* 映射的结束桶号 */

	int m_maxsize; /* 数据大小 */
	char * m_data; /* 映射的数据区 */
	unsigned char m_dirty; /* 是否为脏数据 */
	unsigned char m_attach; /* 外联缓冲 */

public:
	CBFile(int buflen = 1024*1024, int max_size = 0);
	~CBFile();

private:
    /*
     * Function: IsValidData
     * Desc: 是否存在有效数据
     * In: 
     *     unsigned int    site    位置
     * Out: 
     *     none
     * Return code: 
     *     1                   是
     *     0                   否
     */
	unsigned char IsValidData(unsigned int site);

    /*
     * Function: IsValidData
     * Desc: 是否存在有效数据
     * In: 
     *     unsigned int    site    位置
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int SetValidData(unsigned int site, unsigned char bit);

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
	int Open(const char * path, int isize, int ilen, enuBFileType openType = E_READ);

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
     *     const void *    data     数据
	 *     int             data_len 数据长度
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Write(unsigned int site, const void * data, int data_len, unsigned char force = 1);

    /*
     * Function: Read
     * Desc: 读数据
     * In: 
     *     none
     * Out: 
     *     void *          data     数据
	 *     int&            data_len 数据长度
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Read(unsigned int site, void * data, int& data_len);

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
};

#pragma pack(1)

typedef struct _tagBFileEx {
	char name_[128]; /* 文件名 */
	unsigned int isize_; /* 节点数 */
	unsigned int ilen_; /* 节点单元长度 */
	unsigned int inode_; /* 当前节点数 */
	unsigned long long addr_; /* 数据地址 */
	struct _tagBInfo {
		unsigned int offset_ : 31; /* 块所在偏移；从0开始计数 */
		unsigned int bit_ : 1; /* 块是否有效 */
	} binfo_[0]; /* 节点信息 */
	/* 数据区 */
} recBFileEx;

#pragma pack()

#define BFILEEX_GET_VALID(info_, site, bit, offset) \
	{ \
		bit = info_[site].bit_; \
		if (bit == 0x01) \
			offset = info_[site].offset_; \
	}

#define BFILEEX_SET_VALID(info_, site, bit, offset) \
	{ \
		info_[site].bit_ = bit; \
		info_[site].offset_ = offset; \
	}

class CBFileEx
{
	int m_fd; /* 文件句柄 */
	enuBFileType m_openType; /* 打开类型 */
	recBFileEx * m_bFile; /* 相关文件信息 */

	int m_buflen; /* 缓冲数据大小 */
	char * m_buf; /* 数据缓冲 */

public:
	CBFileEx(int buflen = 1024*1024);
	~CBFileEx();

private:
    /*
     * Function: GetOffset
     * Desc: 取数据所在偏移
     * In: 
     *     unsigned int    site    位置
     * Out: 
     *     unsigned int&   offset  偏移
     * Return code: 
     *     1                   是
     *     0                   否
     */
	unsigned char GetOffset(unsigned int site, unsigned int& offset);

    /*
     * Function: SetOffset
     * Desc: 设置偏移
     * In: 
     *     unsigned int    site    位置
     *     unsigned char   valid   是否有效
     *     unsigned int    offset  偏移
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int SetOffset(unsigned int site, unsigned char valid, unsigned int offset);

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
	int Open(const char * path, int isize, int ilen, enuBFileType openType = E_READ);

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
     *     const void *    data     数据
	 *     int             data_len 数据长度
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Write(unsigned int site, const void * data, int data_len, unsigned char force = 1);

    /*
     * Function: Read
     * Desc: 读数据
     * In: 
     *     none
     * Out: 
     *     void *          data     数据
	 *     int&            data_len 数据长度
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
	int Read(unsigned int site, void * data, int& data_len);
};

#endif
