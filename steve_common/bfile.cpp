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
int CMBFile::Open(const char * path, int isize, int ilen, enuBFileType openType /* = E_READ */)
{
	char fname[128];

	/* ��ȡ�ļ��� */
	GetFileName(path, fname);

	/* ȷ���ļ��Ƿ���� */
	if (access(path, F_OK))
	{
		/* �ļ������� */
		if (openType == E_READ)
			return -1;

		/* �����ļ� */
		int fd = open64(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		/* ����������� */
		ftruncate(fd, sizeof(recMBFile) + isize * ilen);

		/* д�ļ�ͷ��Ϣ��ӳ�䵽�ڴ� */
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

		/* �������ݵ���Ч��, ӳ�䵽�ڴ� */
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
 * Desc: �ر��ļ�
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMBFile::Close()
{
	/* �ͷ���Դ���رվ�� */
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
 * Desc: ������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     NULL                ����
 *     ��NULL              �ɹ�
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
	/* �����ڴ� */
	m_buf = (char *)malloc(buflen);

	if (max_size > 0)
		m_data = (char *)malloc(max_size * 1024 * 1024);
	else
		m_data = NULL;
}

CBFile::~CBFile()
{
	Close();

	/* �ͷ���Դ */
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
 * Desc: �Ƿ������Ч����
 * In: 
 *     unsigned int    site    λ��
 * Out: 
 *     none
 * Return code: 
 *     1                   ��
 *     0                   ��
 */
unsigned char CBFile::IsValidData(unsigned int site)
{
	unsigned char bit = 0;

	BFILE_GET_VALID(m_bFile->binfo_, site, bit);

	return (bit == 0x01);
}

/*
 * Function: IsValidData
 * Desc: �Ƿ������Ч����
 * In: 
 *     unsigned int    site    λ��
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBFile::SetValidData(unsigned int site, unsigned char bit)
{
	BFILE_SET_VALID(m_bFile->binfo_, site, bit);

	return 0;
}

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
int CBFile::Open(const char * path, int isize, int ilen, enuBFileType openType /* = E_READ */)
{
	char fname[128];

	/* ��ȡ�ļ��� */
	GetFileName(path, fname);

	/* ȷ���ļ��Ƿ���� */
	if (access(path, F_OK))
	{
		/* �ļ������� */
		if (openType == E_READ)
			return -1;

		/* �����ļ� */
		int fd = open64(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		/* ����������� */
		unsigned int blen = ((isize % 8) > 0) ? (isize /8 + 1) : (isize /8);
		ftruncate64(fd, sizeof(recBFile) + blen + (unsigned long long)isize * ilen);

		/* д�ļ�ͷ��Ϣ��ӳ�䵽�ڴ� */
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

		/* �������ݵ���Ч��, ӳ�䵽�ڴ� */
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

	/* ��Ԫ�����Ƿ񳬹����峤�ȣ������������� */
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
 * Desc: �ر��ļ�
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBFile::Close()
{
	/* �ͷ���Դ���رվ�� */
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
 * Desc: ���������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBFile::SetChunk(int start, int end)
{
	if (m_fd < 0)
		return -1;

	/* ȷ�����ݳ����Ƿ񳬹� */
	int len = (end - start) * m_bFile->ilen_;
	if (len > m_maxsize * 1024 * 1024)
		return -1;

	/* ��ȡ���� */
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
 * Desc: д����
 * In: 
 *     const void *    data     ����
 *     int             data_len ���ݳ���
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
			/* ����Ϊ����Ч����*/
			SetValidData(site, 0);
		}

		return -1;
	}

	/* ȷ�������Ƿ��ڻ���ӳ���� */
	if (site >= m_start && site < m_end)
	{
		char * p = m_data + (site - m_start) * m_bFile->ilen_;
		memset(p, 0, m_bFile->ilen_);
		memcpy(p, data, data_len);

		/* ����Ϊ��Ч���� */
		SetValidData(site, 1);

		/* ������������ */
		m_dirty = 1;

		return 0;
	}

	/* ȷ������λ�� */
	unsigned long long addr = m_bFile->addr_ + (unsigned long long)site * m_bFile->ilen_;

	lseek64(m_fd, addr, SEEK_SET);

	/* д���� */
	memset(m_buf, 0, m_bFile->ilen_);
	memcpy(m_buf, data, data_len);

	write(m_fd, m_buf, m_bFile->ilen_);

	/* ����Ϊ��Ч���� */
	SetValidData(site, 1);

	return 0;
}

/*
 * Function: Read
 * Desc: ������
 * In: 
 *     none
 * Out: 
 *     void *          data     ����
 *     int&            data_len ���ݳ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBFile::Read(unsigned int site, void * data, int& data_len)
{
	if ((unsigned int)data_len < m_bFile->ilen_ || !IsValidData(site))
	{
		data_len = 0;
		return -1;
	}

	/* ȷ�������Ƿ��ڻ���ӳ���� */
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
 * Desc: ��ʱ��������д��ʵ��������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBFile::Flush()
{
	/* ���ݷ��࣬����Ҫ���� */
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

	/* ������λ */
	m_start = m_end = 0;

	return 0;
}

/*
 * Function: Attach
 * Desc: ʹ�������Ļ���
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
 * Desc: ��ʱ��������д��ʵ��������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
	/* �����ڴ� */
	m_buf = (char *)malloc(buflen);
}

CBFileEx::~CBFileEx()
{
	Close();

	/* �ͷ���Դ */
	if (m_buf)
	{
		free(m_buf);
		m_buf = NULL;
	}
}

/*
 * Function: GetOffset
 * Desc: ȡ��������ƫ��
 * In: 
 *     unsigned int    site    λ��
 * Out: 
 *     unsigned int&   offset  ƫ��
 * Return code: 
 *     1                   ��
 *     0                   ��
 */
unsigned char CBFileEx::GetOffset(unsigned int site, unsigned int& offset)
{
	unsigned char bit = 0;

	BFILEEX_GET_VALID(m_bFile->binfo_, site, bit, offset);

	return (bit == 0x01);
}

/*
 * Function: SetOffset
 * Desc: ����ƫ��
 * In: 
 *     unsigned int    site    λ��
 *     unsigned char   valid   �Ƿ���Ч
 *     unsigned int    offset  ƫ��
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBFileEx::SetOffset(unsigned int site, unsigned char valid, unsigned int offset)
{
	BFILEEX_SET_VALID(m_bFile->binfo_, site, valid, offset);

	return 0;
}

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
int CBFileEx::Open(const char * path, int isize, int ilen, enuBFileType openType /* = E_READ */)
{
	char fname[128];

	/* ��ȡ�ļ��� */
	GetFileName(path, fname);

	/* ȷ���ļ��Ƿ���� */
	if (access(path, F_OK))
	{
		/* �ļ������� */
		if (openType == E_READ)
			return -1;

		/* �����ļ� */
		int fd = open64(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		/* ����������� */
		ftruncate(fd, sizeof(recBFileEx) + sizeof(struct _tagBFileEx::_tagBInfo) * isize);

		/* д�ļ�ͷ��Ϣ��ӳ�䵽�ڴ� */
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

		/* �������ݵ���Ч��, ӳ�䵽�ڴ� */
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

	/* ��Ԫ�����Ƿ񳬹����峤�ȣ������������� */
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
 * Desc: �ر��ļ�
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBFileEx::Close()
{
	/* �ͷ���Դ���رվ�� */
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
 * Desc: д����
 * In: 
 *     const void *    data     ����
 *     int             data_len ���ݳ���
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
			/* ����Ϊ����Ч����*/
			SetOffset(site, 0, 0);
		}

		return -1;
	}

	/* ȷ������λ�� */
	unsigned int ichunk = m_bFile->ilen_ + sizeof(unsigned int);
	unsigned int offset = 0;
	unsigned char exist = GetOffset(site, offset);
	offset = (exist == 0) ? m_bFile->inode_ : offset;
	unsigned long long addr = m_bFile->addr_ + (unsigned long long)offset * ichunk;

	lseek64(m_fd, addr, SEEK_SET);

	/* д���� */
	memset(m_buf, 0, ichunk);

	char * p = m_buf;
	memcpy(p, &site, sizeof(unsigned int));
	p += sizeof(unsigned int);
	memcpy(p, data, data_len);

	write(m_fd, m_buf, ichunk);

	/* ����Ϊ��Ч���� */
	if (!exist)
	{
		SetOffset(site, 1, m_bFile->inode_);
		m_bFile->inode_++;
	}

	return 0;
}

/*
 * Function: Read
 * Desc: ������
 * In: 
 *     none
 * Out: 
 *     void *          data     ����
 *     int&            data_len ���ݳ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBFileEx::Read(unsigned int site, void * data, int& data_len)
{
	unsigned int offset = 0;
	if ((unsigned int)data_len < m_bFile->ilen_ || !GetOffset(site, offset))
	{
		data_len = 0;
		return -1;
	}

	/* ��ȡ���� */
	unsigned int ichunk = m_bFile->ilen_ + sizeof(unsigned int);
	unsigned long long addr = m_bFile->addr_ + (unsigned long long)offset * ichunk;

	lseek64(m_fd, addr, SEEK_SET);

	int n = read(m_fd, m_buf, ichunk);
	if (n <= 0)
	{
		data_len = 0;
		return -1;
	}

	/* ȷ��site�Ƿ�ƥ�� */
	char * p = m_buf;
	unsigned int tmp = 0;
	memcpy(&tmp, p, sizeof(unsigned int));

	if (tmp != site)
	{
		data_len = 0;
		return -1;
	}

	/* ȡ���� */
	p += sizeof(unsigned int);
	memcpy(data, p, m_bFile->ilen_);

	data_len = m_bFile->ilen_;

	return 0;
}
