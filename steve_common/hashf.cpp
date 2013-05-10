/*
 * @File: hashf.cpp
 * @Desc: impliment file of Hash File
 * @Author: stevetang
 * @History:
 *      2008-12-16   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "common.h"
#include "hashf.h"

CHashF::CHashF(pHashFGetBucketID get_bucket_id, pHashFCmpKey cmp_key, int buflen /* = 1024 * 1024 */) : 
	m_get_bucket_id(get_bucket_id), m_cmp_key(cmp_key), m_fd(-1), m_buflen(buflen), m_enum_r(0), m_enum_w(0)
{
	memset(&m_rHashF, 0, sizeof(recHashF));

	m_buf = (char *)malloc(buflen);

	m_hashfNode = (recHashFNode *)malloc((buflen/(sizeof(recHashFNode)-sizeof(char *))) * sizeof(recHashFNode));
}

CHashF::~CHashF()
{
	/* 关闭文件 */
	Close();

	/* 释放资源 */
	if (m_buf)
	{
		free(m_buf);
		m_buf = NULL;
	}

	if (m_hashfNode)
	{
		free(m_hashfNode);
		m_hashfNode = NULL;
	}
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
int CHashF::Open(const char * path, int bucket /* = 7 */, enuHashFType openType /* = E_READ */)
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
		strcpy(m_rHashF.name, fname);
		m_rHashF.b_size = bucket;
		m_rHashF.addr = sizeof(recHashF) + sizeof(unsigned long long) * bucket;

		m_fd = open64(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (m_fd == -1)
			return -1;

		/* 清空所有数据 */
		ftruncate(m_fd, sizeof(recHashF) + m_rHashF.b_size * sizeof(unsigned long long));

		/* 写文件头 */
		lseek64(m_fd, 0, SEEK_SET);
		write(m_fd, &m_rHashF, sizeof(recHashF));
	}
	else
	{
		if (openType == E_READ)
			m_fd = open64(path, O_RDONLY);
		else
			m_fd = open64(path, O_RDWR);

		/* 检验数据的有效性 */
		read(m_fd, &m_rHashF, sizeof(recHashF));

		if (strcmp(m_rHashF.name, fname))
			return -1;

		if (m_rHashF.addr != (sizeof(recHashF) + sizeof(unsigned long long) * m_rHashF.b_size))
			return -1;
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
int CHashF::Close()
{
	if (m_fd < 0)
		return -1;

	close(m_fd);

	return 0;
}

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
int CHashF::Write(const void * key, int key_len, const void * data, int data_len)
{
	/* 确认桶的位置 */
	char buf[10240];
	int site = m_get_bucket_id(key) % m_rHashF.b_size;

	unsigned long long addr = sizeof(recHashF) + sizeof(unsigned long long) * site;

	lseek64(m_fd, addr, SEEK_SET);

	/* 读取下一节点地址 */
	unsigned long long prev = 0;
	unsigned long long next = 0;
	unsigned long long cur = 0;

	read(m_fd, &next, sizeof(unsigned long long));

	unsigned long long head = next;

	/* 检查是否有相同节点 */
	while (next)
	{
		/* 确认节点位置 */
		cur = lseek64(m_fd, next, SEEK_SET);

		/* 读取数据 */
		read(m_fd, buf, sizeof(buf));

		char * ptr = buf;

		/* 删除标志 */
		unsigned char del = *((unsigned char *)ptr);
		ptr++;

		/* 下一节点 */
		next = *((unsigned long long *)ptr);
		ptr += sizeof(unsigned long long);

		/* 读取键值长度 */
		unsigned int klen = *((unsigned int *)ptr);
		ptr += sizeof(unsigned int);

		if (m_cmp_key(key, ptr) == 0)
		{
			/* 键值相同 */
			ptr += klen;

			unsigned int dlen = *((unsigned int *)ptr);

			if (dlen >= (unsigned int)data_len)
			{
				/* 原有数据长度大于现有长度，写入数据 */
				memcpy(ptr, &data_len, sizeof(int));
				ptr += sizeof(unsigned int);

				/* 移到原有位置修改 */
				lseek64(m_fd, cur, SEEK_SET);

				write(m_fd, buf, ptr-buf);
				write(m_fd, data, data_len);

				return 0;
			}

			/* 删除节点 */
			lseek64(m_fd, cur, SEEK_SET);

			del = 1;
			write(m_fd, &del, sizeof(unsigned char));

			if (prev)
			{
				lseek64(m_fd, prev+sizeof(unsigned char), SEEK_SET);

				write(m_fd, &next, sizeof(unsigned long long));
			}
			else
			{
				head = next; /* 第一个节点 */
			}

			break;
		}

		prev = cur;
	}

	cur = lseek64(m_fd, 0, SEEK_END);

	char * ptr = buf;

	/* 删除标志 */
	*ptr = 0x00;
	ptr++;

	/* 下一节点地址 */
	memcpy(ptr, &head, sizeof(unsigned long long));
	ptr += sizeof(unsigned long long);

	/* 写键值长度 */
	memcpy(ptr, &key_len, sizeof(int));
	ptr += sizeof(unsigned int);

	/* 写键值 */
	memcpy(ptr, key, key_len);
	ptr += key_len;

	/* 写数据长度 */
	memcpy(ptr, &data_len, sizeof(int));
	ptr += sizeof(unsigned int);

	/* 写数据 */
	write(m_fd, buf, ptr-buf);
	write(m_fd, data, data_len);

	/* 写头结点 */
	lseek64(m_fd, addr, SEEK_SET);

	write(m_fd, &cur, sizeof(unsigned long long));

	return 0;
}

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
int CHashF::Read(const void * key, int key_len, void * data, int& data_len)
{
	char buf[10240];
	int site = m_get_bucket_id(key) % m_rHashF.b_size;

	unsigned long long addr = sizeof(recHashF) + sizeof(unsigned long long) * site;

	lseek64(m_fd, addr, SEEK_SET);

	/* 读取下一节点地址 */
	unsigned long long next = 0;
	unsigned long long cur = 0;

	read(m_fd, &next, sizeof(unsigned long long));

	/* 检查是否有相同节点 */
	while (next)
	{
		cur = lseek64(m_fd, next, SEEK_SET);

		/* 读取数据 */
		read(m_fd, buf, sizeof(buf));

		char * ptr = buf;

		/* 删除标志 */
		ptr++;

		/* 下一节点 */
		next = *((unsigned long long *)ptr);
		ptr += sizeof(unsigned long long);

		int klen = *((int *)ptr);
		ptr += sizeof(int);

		if (m_cmp_key(key, ptr) == 0)
		{
			ptr += klen;

			int dlen = *((int *)ptr);
			ptr += sizeof(int);

			if (dlen > data_len)
			{
				data_len = 0;
				return -2;
			}

			if ((unsigned int)(dlen + (ptr - buf)) > sizeof(buf))
			{
				int len = sizeof(buf) - (ptr - buf);
				memcpy(data, ptr, len);
				read(m_fd, ((char *)data)+len, dlen-len);
			}
			else
			{
				memcpy(data, ptr, dlen);
			}

			return 0;
		}
	}

	return -1;
}

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
int CHashF::EnumBegin()
{
	/* 设置读写位置 */
	m_enum_r = m_enum_w = m_rHashF.addr;

	return 0;
}

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
int CHashF::EnumRead(recHashFNode * &rHashFNode, int &num)
{
	/* 偏移到读取数据位置 */
	lseek64(m_fd, m_enum_r, SEEK_SET);

	/* 读取数据 */
	int n = read(m_fd, m_buf, m_buflen);
	if (n <= 0)
		return -1;

	/* 分析有效数据 */
	char * ptr = m_buf;
	int i = 0;
	while ((ptr - m_buf) < n)
	{
		char * p = ptr;

		/* 跳过删除节点标记 */
		p++;

		/* 下一节点 */
		memcpy(&m_hashfNode[i].next, p, sizeof(unsigned long long));
		p += sizeof(unsigned long long);

		/* 键值长度 */
		memcpy(&m_hashfNode[i].key_len, p, sizeof(unsigned int));
		p += sizeof(unsigned int);

		/* 确认键值是否全 */
		if ((n - (p - m_buf)) < (int)m_hashfNode[i].key_len)
			break;
		m_hashfNode[i].key = p;
		p += m_hashfNode[i].key_len;

		/* 数据长度 */
		memcpy(&m_hashfNode[i].data_len, p, sizeof(unsigned int));
		p += sizeof(unsigned int);

		/* 确认数据是否全 */
		if ((n - (p - m_buf)) < (int)m_hashfNode[i].data_len)
			break;
		m_hashfNode[i].data = p;
		p += m_hashfNode[i].data_len;

		ptr = p;
		i++;
	}

	/* 设置下次数据读起始位置 */
	m_enum_r += (ptr - m_buf);

	rHashFNode = m_hashfNode;
	num = i;

	return 0;
}

/*
 * Function: EnumWrite
 * Desc: 写回枚举数据
 * In: 
 *     recHashFNode * rHashFNode  数据节点
 *     int            num         节点数目
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CHashF::EnumWrite(recHashFNode * rHashFNode, int num)
{
	/* 确认数据有效性 */
	char * ptr = m_buf;
	for (int i=0; i<num; i++)
	{
		/* 下一节点 */
		unsigned long long next = 0;
		memcpy(&next, ptr, sizeof(unsigned long long));
		if (next != rHashFNode[i].next)
			return -1;
		ptr += sizeof(unsigned long long);

		/* 键值长度 */
		unsigned int key_len = 0;
		memcpy(&key_len, ptr, sizeof(unsigned int));
		if (key_len != rHashFNode[i].key_len)
			return -1;
		ptr += sizeof(unsigned int);

		/* 键值 */
		ptr += rHashFNode[i].key_len;

		/* 数据长度 */
		unsigned int data_len = 0;
		memcpy(&data_len, ptr, sizeof(unsigned int));
		if (data_len != rHashFNode[i].data_len)
			return -1;
		ptr += sizeof(unsigned int);

		/* 数据 */
		ptr += rHashFNode[i].data_len;
	}

	/* 数据长度已变化 */
	int length = ptr - m_buf;
	if (length > (int)(m_enum_r - m_enum_w))
		return -1;

	/* 偏移到写数据位置 */
	lseek64(m_fd, m_enum_w, SEEK_SET);

	/* 写数据 */
	write(m_fd, m_buf, length);

	m_enum_w += length;

	return 0;
}

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
int CHashF::EnumClose()
{
	return 0;
}
