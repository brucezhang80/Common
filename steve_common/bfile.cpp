/*
 * @File: bhashf.cpp
 * @Desc: impliment file of Bitmap File
 * @Author: stevetang
 * @History:
 *      2009-04-28   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "common.h"
#include "bfile.h"

CMBFile::CMBFile() : m_fd(-1), m_openType(E_READ), m_bFile(NULL)
{
}

CMBFile::~CMBFile()
{
	Close();
}

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
int CMBFile::Open(const char * path, int isize, int ilen, enuBFileType openType /* = E_READ */)
{
	char fname[128];

	/* 获取文件名 */
	GetFileName(path, fname);

	/* 确认文件是否存在 */
	if (access(path, F_OK))
	{
		/* 文件不存在 */
		if (openType == E_READ)
			return -1;

		/* 创建文件 */
		int fd = open64(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		/* 清空所有数据 */
		ftruncate(fd, sizeof(recMBFile) + isize * ilen);

		/* 写文件头信息，映射到内存 */
		recMBFile * bFile = (recMBFile *)mmap(NULL, sizeof(recMBFile) + isize * ilen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (bFile == (recMBFile *)-1)
		{
			close(fd);
			return -1;
		}

		strcpy(bFile->name_, fname);
		bFile->isize_ = isize;
		bFile->ilen_ = ilen;

		m_fd = fd;
		m_bFile = bFile;
	}
	else
	{
		int fd = -1;
		recMBFile * bFile = NULL;

		if (openType == E_READ)
		{
			fd = open64(path, O_RDONLY);
			bFile = (recMBFile *)mmap(NULL, sizeof(recMBFile) + isize * ilen, PROT_READ, MAP_SHARED, fd, 0);
		}
		else
		{
			fd = open64(path, O_RDWR);
			bFile = (recMBFile *)mmap(NULL, sizeof(recMBFile) + isize * ilen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		}

		/* 检验数据的有效性, 映射到内存 */
		if (bFile == (recMBFile *)-1)
		{
			close(fd);
			return -1;
		}

		if (strcmp(bFile->name_, fname) && 
			bFile->isize_ != (unsigned int)isize && 
			bFile->ilen_ != (unsigned int)ilen)
		{
			close(fd);
			return -1;
		}

		m_fd = fd;
		m_bFile = bFile;
	}

	m_openType = openType;

	return 0;
}

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
int CMBFile::Close()
{
	/* 释放资源，关闭句柄 */
	if (m_bFile)
	{
		munmap(m_bFile, sizeof(recMBFile) + m_bFile->isize_ * m_bFile->ilen_);

		m_bFile = NULL;
	}

	if (m_fd > 0)
	{
		close(m_fd);
		m_fd = -1;
	}

	return 0;
}

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
void * CMBFile::Read(unsigned int site)
{
	if (site >= m_bFile->isize_)
		return NULL;

	return (m_bFile->data_ + site * m_bFile->ilen_);
}

CBFile::CBFile(int buflen /* = 1024*1024 */, int max_size /* = 0 */) : 
m_fd(-1), m_openType(E_READ), m_bFile(NULL), m_buflen(buflen), m_start(0), 
m_end(0), m_maxsize(max_size), m_dirty(0), m_attach(0)
{
	/* 分配内存 */
	m_buf = (char *)malloc(buflen);

	if (max_size > 0)
		m_data = (char *)malloc(max_size * 1024 * 1024);
	else
		m_data = NULL;
}

CBFile::~CBFile()
{
	Close();

	/* 释放资源 */
	if (m_data && m_attach == 0)
	{
		free(m_data);
		m_data = NULL;
	}

	if (m_buf)
	{
		free(m_buf);
		m_buf = NULL;
	}
}

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
unsigned char CBFile::IsValidData(unsigned int site)
{
	unsigned char bit = 0;

	BFILE_GET_VALID(m_bFile->binfo_, site, bit);

	return (bit == 0x01);
}

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
int CBFile::SetValidData(unsigned int site, unsigned char bit)
{
	BFILE_SET_VALID(m_bFile->binfo_, site, bit);

	return 0;
}

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
int CBFile::Open(const char * path, int isize, int ilen, enuBFileType openType /* = E_READ */)
{
	char fname[128];

	/* 获取文件名 */
	GetFileName(path, fname);

	/* 确认文件是否存在 */
	if (access(path, F_OK))
	{
		/* 文件不存在 */
		if (openType == E_READ)
			return -1;

		/* 创建文件 */
		int fd = open64(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		/* 清空所有数据 */
		unsigned int blen = ((isize % 8) > 0) ? (isize /8 + 1) : (isize /8);
		ftruncate64(fd, sizeof(recBFile) + blen + (unsigned long long)isize * ilen);

		/* 写文件头信息，映射到内存 */
		recBFile * bFile = (recBFile *)mmap(NULL, sizeof(recBFile) + blen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (bFile == (recBFile *)-1)
		{
			close(fd);
			return -1;
		}

		strcpy(bFile->name_, fname);
		bFile->isize_ = isize;
		bFile->ilen_ = ilen;
		bFile->addr_ = sizeof(recBFile) + blen;

		m_fd = fd;
		m_bFile = bFile;
	}
	else
	{
		int fd = -1;

		/* 检验数据的有效性, 映射到内存 */
		unsigned int blen = ((isize % 8) > 0) ? (isize /8 + 1) : (isize /8);
		recBFile * bFile = NULL;
		if (openType == E_READ)
		{
			fd = open64(path, O_RDONLY);
			bFile = (recBFile *)mmap(NULL, sizeof(recBFile) + blen, PROT_READ, MAP_SHARED, fd, 0);
		}
		else
		{
			fd = open64(path, O_RDWR);
			bFile = (recBFile *)mmap(NULL, sizeof(recBFile) + blen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		}

		if (bFile == (recBFile *)-1)
		{
			close(fd);
			return -1;
		}

		m_fd = fd;
		m_bFile = bFile;

		if (strcmp(m_bFile->name_, fname))
			return -1;

		if (openType == E_READ)
			return 0;
	}

	m_openType = openType;

	/* 单元长度是否超过缓冲长度，超过重新申请 */
	if (m_bFile->ilen_ > (unsigned int)m_buflen)
	{
		if (m_buf)
		{
			free(m_buf);
			m_buf = NULL;
		}

		m_buf = (char *)malloc(m_bFile->ilen_);
		if (m_buf == NULL)
			return -1;
		m_buflen = m_bFile->ilen_;
	}

	return 0;
}

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
int CBFile::Close()
{
	/* 释放资源，关闭句柄 */
	Flush();

	if (m_bFile)
	{
		unsigned int blen = ((m_bFile->isize_ % 8) > 0) ? (m_bFile->isize_ /8 + 1) : (m_bFile->isize_ /8);
		munmap(m_bFile, sizeof(recBFile) + blen);

		m_bFile = NULL;
	}

	if (m_fd > 0)
	{
		close(m_fd);
		m_fd = -1;
	}

	return 0;
}

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
int CBFile::SetChunk(int start, int end)
{
	if (m_fd < 0)
		return -1;

	/* 确认数据长度是否超过 */
	int len = (end - start) * m_bFile->ilen_;
	if (len > m_maxsize * 1024 * 1024)
		return -1;

	/* 读取数据 */
	unsigned long long addr = m_bFile->addr_ + (unsigned long long)start * m_bFile->ilen_;
	lseek64(m_fd, addr, SEEK_SET);

	char * p = m_data;
	while (len > 0)
	{
		int r = read(m_fd, p, len);
		if (r <= 0)
			break;

		len -= r;
		p += r;
	}

	m_start = start;
	m_end = end;

	return 0;
}

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
int CBFile::Write(unsigned int site, const void * data, int data_len, unsigned char force /* = 1 */)
{
	if (m_openType == E_READ)
	{
		return -1;
	}

	if ((unsigned int)data_len > m_bFile->ilen_)
	{
		if (force)
		{
			/* 设置为非有效数据*/
			SetValidData(site, 0);
		}

		return -1;
	}

	/* 确认数据是否在缓冲映射区 */
	if (site >= m_start && site < m_end)
	{
		char * p = m_data + (site - m_start) * m_bFile->ilen_;
		memset(p, 0, m_bFile->ilen_);
		memcpy(p, data, data_len);

		/* 设置为有效数据 */
		SetValidData(site, 1);

		/* 设置数据已脏 */
		m_dirty = 1;

		return 0;
	}

	/* 确定数据位置 */
	unsigned long long addr = m_bFile->addr_ + (unsigned long long)site * m_bFile->ilen_;

	lseek64(m_fd, addr, SEEK_SET);

	/* 写数据 */
	memset(m_buf, 0, m_bFile->ilen_);
	memcpy(m_buf, data, data_len);

	write(m_fd, m_buf, m_bFile->ilen_);

	/* 设置为有效数据 */
	SetValidData(site, 1);

	return 0;
}

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
int CBFile::Read(unsigned int site, void * data, int& data_len)
{
	if ((unsigned int)data_len < m_bFile->ilen_ || !IsValidData(site))
	{
		data_len = 0;
		return -1;
	}

	/* 确认数据是否在缓冲映射区 */
	if (site >= m_start && site < m_end)
	{
		char * p = m_data + (site - m_start) * m_bFile->ilen_;
		memcpy(data, p, m_bFile->ilen_);
		data_len = m_bFile->ilen_;
		return 0;
	}

	unsigned long long addr = m_bFile->addr_ + (unsigned long long)site * m_bFile->ilen_;

	lseek64(m_fd, addr, SEEK_SET);

	data_len = read(m_fd, data, m_bFile->ilen_);

	return 0;
}

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
int CBFile::Flush()
{
	/* 数据非脏，不需要更新 */
	if (m_dirty == 0 || m_data == NULL)
		return 0;

	int len = (m_end - m_start) * m_bFile->ilen_;

	unsigned long long addr = m_bFile->addr_ + (unsigned long long)m_start * m_bFile->ilen_;
	lseek64(m_fd, addr, SEEK_SET);

	char * p = m_data;

	while (len > 0)
	{
		int r = write(m_fd, p, len);
		if (r <= 0)
			return -1;

		len -= r;
		p += r;
	}

	m_dirty = 0;

	/* 区间置位 */
	m_start = m_end = 0;

	return 0;
}

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
int CBFile::Attach(int maxsize, char * data)
{
	if (m_data || m_maxsize > 0)
		return -1;

	m_maxsize = maxsize;
	m_data = data;

	m_attach = 1;

	return 0;
}

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
int CBFile::Detach()
{
	if (!m_attach)
		return -1;

	m_maxsize = 0;
	m_data = NULL;

	m_attach = 0;

	return 0;
}

CBFileEx::CBFileEx(int buflen /* = 1024*1024 */) : 
m_fd(-1), m_openType(E_READ), m_bFile(NULL), m_buflen(buflen)
{
	/* 分配内存 */
	m_buf = (char *)malloc(buflen);
}

CBFileEx::~CBFileEx()
{
	Close();

	/* 释放资源 */
	if (m_buf)
	{
		free(m_buf);
		m_buf = NULL;
	}
}

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
unsigned char CBFileEx::GetOffset(unsigned int site, unsigned int& offset)
{
	unsigned char bit = 0;

	BFILEEX_GET_VALID(m_bFile->binfo_, site, bit, offset);

	return (bit == 0x01);
}

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
int CBFileEx::SetOffset(unsigned int site, unsigned char valid, unsigned int offset)
{
	BFILEEX_SET_VALID(m_bFile->binfo_, site, valid, offset);

	return 0;
}

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
int CBFileEx::Open(const char * path, int isize, int ilen, enuBFileType openType /* = E_READ */)
{
	char fname[128];

	/* 获取文件名 */
	GetFileName(path, fname);

	/* 确认文件是否存在 */
	if (access(path, F_OK))
	{
		/* 文件不存在 */
		if (openType == E_READ)
			return -1;

		/* 创建文件 */
		int fd = open64(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		/* 清空所有数据 */
		ftruncate(fd, sizeof(recBFileEx) + sizeof(struct _tagBFileEx::_tagBInfo) * isize);

		/* 写文件头信息，映射到内存 */
		recBFileEx * bFile = (recBFileEx *)mmap(NULL, sizeof(recBFileEx) + sizeof(struct _tagBFileEx::_tagBInfo) * isize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (bFile == (recBFileEx *)-1)
		{
			close(fd);
			return -1;
		}

		strcpy(bFile->name_, fname);
		bFile->isize_ = isize;
		bFile->ilen_ = ilen;
		bFile->inode_ = 0;
		bFile->addr_ = sizeof(recBFileEx) + sizeof(struct _tagBFileEx::_tagBInfo) * isize;

		m_fd = fd;
		m_bFile = bFile;
	}
	else
	{
		int fd = -1;
		recBFileEx * bFile = NULL;

		if (openType == E_READ)
		{
			fd = open64(path, O_RDONLY);
			if (fd == -1)
				return -1;
			bFile = (recBFileEx *)mmap(NULL, sizeof(recBFileEx) + sizeof(struct _tagBFileEx::_tagBInfo) * isize, PROT_READ, MAP_SHARED, fd, 0);
		}
		else
		{
			fd = open64(path, O_RDWR);
			if (fd == -1)
				return -1;
			bFile = (recBFileEx *)mmap(NULL, sizeof(recBFileEx) + sizeof(struct _tagBFileEx::_tagBInfo) * isize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		}

		/* 检验数据的有效性, 映射到内存 */
		if (bFile == (recBFileEx *)-1)
		{
			close(fd);
			return -1;
		}

		m_fd = fd;
		m_bFile = bFile;

		if (strcmp(m_bFile->name_, fname))
			return -1;
	}

	m_openType = openType;

	/* 单元长度是否超过缓冲长度，超过重新申请 */
	if (m_bFile->ilen_ > (unsigned int)m_buflen)
	{
		if (m_buf)
		{
			free(m_buf);
			m_buf = NULL;
		}

		m_buf = (char *)malloc(m_bFile->ilen_);
		if (m_buf == NULL)
			return -1;
		m_buflen = m_bFile->ilen_;
	}

	return 0;
}

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
int CBFileEx::Close()
{
	/* 释放资源，关闭句柄 */
	if (m_bFile)
	{
		munmap(m_bFile, sizeof(recBFileEx) + sizeof(struct _tagBFileEx::_tagBInfo) * m_bFile->isize_);

		m_bFile = NULL;
	}

	if (m_fd > 0)
	{
		close(m_fd);
		m_fd = -1;
	}

	return 0;
}

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
int CBFileEx::Write(unsigned int site, const void * data, int data_len, unsigned char force /* = 1 */)
{
	if (m_openType == E_READ)
	{
		return -1;
	}

	if ((unsigned int)data_len > m_bFile->ilen_)
	{
		if (force)
		{
			/* 设置为非有效数据*/
			SetOffset(site, 0, 0);
		}

		return -1;
	}

	/* 确定数据位置 */
	unsigned int ichunk = m_bFile->ilen_ + sizeof(unsigned int);
	unsigned int offset = 0;
	unsigned char exist = GetOffset(site, offset);
	offset = (exist == 0) ? m_bFile->inode_ : offset;
	unsigned long long addr = m_bFile->addr_ + (unsigned long long)offset * ichunk;

	lseek64(m_fd, addr, SEEK_SET);

	/* 写数据 */
	memset(m_buf, 0, ichunk);

	char * p = m_buf;
	memcpy(p, &site, sizeof(unsigned int));
	p += sizeof(unsigned int);
	memcpy(p, data, data_len);

	write(m_fd, m_buf, ichunk);

	/* 设置为有效数据 */
	if (!exist)
	{
		SetOffset(site, 1, m_bFile->inode_);
		m_bFile->inode_++;
	}

	return 0;
}

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
int CBFileEx::Read(unsigned int site, void * data, int& data_len)
{
	unsigned int offset = 0;
	if ((unsigned int)data_len < m_bFile->ilen_ || !GetOffset(site, offset))
	{
		data_len = 0;
		return -1;
	}

	/* 读取数据 */
	unsigned int ichunk = m_bFile->ilen_ + sizeof(unsigned int);
	unsigned long long addr = m_bFile->addr_ + (unsigned long long)offset * ichunk;

	lseek64(m_fd, addr, SEEK_SET);

	int n = read(m_fd, m_buf, ichunk);
	if (n <= 0)
	{
		data_len = 0;
		return -1;
	}

	/* 确认site是否匹配 */
	char * p = m_buf;
	unsigned int tmp = 0;
	memcpy(&tmp, p, sizeof(unsigned int));

	if (tmp != site)
	{
		data_len = 0;
		return -1;
	}

	/* 取数据 */
	p += sizeof(unsigned int);
	memcpy(data, p, m_bFile->ilen_);

	data_len = m_bFile->ilen_;

	return 0;
}
