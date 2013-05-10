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
	/* �ر��ļ� */
	Close();

	/* �ͷ���Դ */
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
int CHashF::Open(const char * path, int bucket /* = 7 */, enuHashFType openType /* = E_READ */)
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
		strcpy(m_rHashF.name, fname);
		m_rHashF.b_size = bucket;
		m_rHashF.addr = sizeof(recHashF) + sizeof(unsigned long long) * bucket;

		m_fd = open64(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (m_fd == -1)
			return -1;

		/* ����������� */
		ftruncate(m_fd, sizeof(recHashF) + m_rHashF.b_size * sizeof(unsigned long long));

		/* д�ļ�ͷ */
		lseek64(m_fd, 0, SEEK_SET);
		write(m_fd, &m_rHashF, sizeof(recHashF));
	}
	else
	{
		if (openType == E_READ)
			m_fd = open64(path, O_RDONLY);
		else
			m_fd = open64(path, O_RDWR);

		/* �������ݵ���Ч�� */
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
 * Desc: �ر��ļ�
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     int             key_len  ������
 *     const void *    data     ����
 *     int             data_len ���ݳ���
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CHashF::Write(const void * key, int key_len, const void * data, int data_len)
{
	/* ȷ��Ͱ��λ�� */
	char buf[10240];
	int site = m_get_bucket_id(key) % m_rHashF.b_size;

	unsigned long long addr = sizeof(recHashF) + sizeof(unsigned long long) * site;

	lseek64(m_fd, addr, SEEK_SET);

	/* ��ȡ��һ�ڵ��ַ */
	unsigned long long prev = 0;
	unsigned long long next = 0;
	unsigned long long cur = 0;

	read(m_fd, &next, sizeof(unsigned long long));

	unsigned long long head = next;

	/* ����Ƿ�����ͬ�ڵ� */
	while (next)
	{
		/* ȷ�Ͻڵ�λ�� */
		cur = lseek64(m_fd, next, SEEK_SET);

		/* ��ȡ���� */
		read(m_fd, buf, sizeof(buf));

		char * ptr = buf;

		/* ɾ����־ */
		unsigned char del = *((unsigned char *)ptr);
		ptr++;

		/* ��һ�ڵ� */
		next = *((unsigned long long *)ptr);
		ptr += sizeof(unsigned long long);

		/* ��ȡ��ֵ���� */
		unsigned int klen = *((unsigned int *)ptr);
		ptr += sizeof(unsigned int);

		if (m_cmp_key(key, ptr) == 0)
		{
			/* ��ֵ��ͬ */
			ptr += klen;

			unsigned int dlen = *((unsigned int *)ptr);

			if (dlen >= (unsigned int)data_len)
			{
				/* ԭ�����ݳ��ȴ������г��ȣ�д������ */
				memcpy(ptr, &data_len, sizeof(int));
				ptr += sizeof(unsigned int);

				/* �Ƶ�ԭ��λ���޸� */
				lseek64(m_fd, cur, SEEK_SET);

				write(m_fd, buf, ptr-buf);
				write(m_fd, data, data_len);

				return 0;
			}

			/* ɾ���ڵ� */
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
				head = next; /* ��һ���ڵ� */
			}

			break;
		}

		prev = cur;
	}

	cur = lseek64(m_fd, 0, SEEK_END);

	char * ptr = buf;

	/* ɾ����־ */
	*ptr = 0x00;
	ptr++;

	/* ��һ�ڵ��ַ */
	memcpy(ptr, &head, sizeof(unsigned long long));
	ptr += sizeof(unsigned long long);

	/* д��ֵ���� */
	memcpy(ptr, &key_len, sizeof(int));
	ptr += sizeof(unsigned int);

	/* д��ֵ */
	memcpy(ptr, key, key_len);
	ptr += key_len;

	/* д���ݳ��� */
	memcpy(ptr, &data_len, sizeof(int));
	ptr += sizeof(unsigned int);

	/* д���� */
	write(m_fd, buf, ptr-buf);
	write(m_fd, data, data_len);

	/* дͷ��� */
	lseek64(m_fd, addr, SEEK_SET);

	write(m_fd, &cur, sizeof(unsigned long long));

	return 0;
}

/*
 * Function: Read
 * Desc: ������
 * In: 
 *     const void *    key      ��ֵ
 *     int             key_len  ������
 * Out: 
 *     void *          data     ����
 *     int&            data_len ���ݳ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CHashF::Read(const void * key, int key_len, void * data, int& data_len)
{
	char buf[10240];
	int site = m_get_bucket_id(key) % m_rHashF.b_size;

	unsigned long long addr = sizeof(recHashF) + sizeof(unsigned long long) * site;

	lseek64(m_fd, addr, SEEK_SET);

	/* ��ȡ��һ�ڵ��ַ */
	unsigned long long next = 0;
	unsigned long long cur = 0;

	read(m_fd, &next, sizeof(unsigned long long));

	/* ����Ƿ�����ͬ�ڵ� */
	while (next)
	{
		cur = lseek64(m_fd, next, SEEK_SET);

		/* ��ȡ���� */
		read(m_fd, buf, sizeof(buf));

		char * ptr = buf;

		/* ɾ����־ */
		ptr++;

		/* ��һ�ڵ� */
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
 * Desc: �����ļ�ö��
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CHashF::EnumBegin()
{
	/* ���ö�дλ�� */
	m_enum_r = m_enum_w = m_rHashF.addr;

	return 0;
}

/*
 * Function: EnumRead
 * Desc: ö������
 * In: 
 *     none
 * Out: 
 *     recHashFNode * &rHashFNode  ���ݽڵ�
 *     int            &num         �ڵ���Ŀ
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CHashF::EnumRead(recHashFNode * &rHashFNode, int &num)
{
	/* ƫ�Ƶ���ȡ����λ�� */
	lseek64(m_fd, m_enum_r, SEEK_SET);

	/* ��ȡ���� */
	int n = read(m_fd, m_buf, m_buflen);
	if (n <= 0)
		return -1;

	/* ������Ч���� */
	char * ptr = m_buf;
	int i = 0;
	while ((ptr - m_buf) < n)
	{
		char * p = ptr;

		/* ����ɾ���ڵ��� */
		p++;

		/* ��һ�ڵ� */
		memcpy(&m_hashfNode[i].next, p, sizeof(unsigned long long));
		p += sizeof(unsigned long long);

		/* ��ֵ���� */
		memcpy(&m_hashfNode[i].key_len, p, sizeof(unsigned int));
		p += sizeof(unsigned int);

		/* ȷ�ϼ�ֵ�Ƿ�ȫ */
		if ((n - (p - m_buf)) < (int)m_hashfNode[i].key_len)
			break;
		m_hashfNode[i].key = p;
		p += m_hashfNode[i].key_len;

		/* ���ݳ��� */
		memcpy(&m_hashfNode[i].data_len, p, sizeof(unsigned int));
		p += sizeof(unsigned int);

		/* ȷ�������Ƿ�ȫ */
		if ((n - (p - m_buf)) < (int)m_hashfNode[i].data_len)
			break;
		m_hashfNode[i].data = p;
		p += m_hashfNode[i].data_len;

		ptr = p;
		i++;
	}

	/* �����´����ݶ���ʼλ�� */
	m_enum_r += (ptr - m_buf);

	rHashFNode = m_hashfNode;
	num = i;

	return 0;
}

/*
 * Function: EnumWrite
 * Desc: д��ö������
 * In: 
 *     recHashFNode * rHashFNode  ���ݽڵ�
 *     int            num         �ڵ���Ŀ
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CHashF::EnumWrite(recHashFNode * rHashFNode, int num)
{
	/* ȷ��������Ч�� */
	char * ptr = m_buf;
	for (int i=0; i<num; i++)
	{
		/* ��һ�ڵ� */
		unsigned long long next = 0;
		memcpy(&next, ptr, sizeof(unsigned long long));
		if (next != rHashFNode[i].next)
			return -1;
		ptr += sizeof(unsigned long long);

		/* ��ֵ���� */
		unsigned int key_len = 0;
		memcpy(&key_len, ptr, sizeof(unsigned int));
		if (key_len != rHashFNode[i].key_len)
			return -1;
		ptr += sizeof(unsigned int);

		/* ��ֵ */
		ptr += rHashFNode[i].key_len;

		/* ���ݳ��� */
		unsigned int data_len = 0;
		memcpy(&data_len, ptr, sizeof(unsigned int));
		if (data_len != rHashFNode[i].data_len)
			return -1;
		ptr += sizeof(unsigned int);

		/* ���� */
		ptr += rHashFNode[i].data_len;
	}

	/* ���ݳ����ѱ仯 */
	int length = ptr - m_buf;
	if (length > (int)(m_enum_r - m_enum_w))
		return -1;

	/* ƫ�Ƶ�д����λ�� */
	lseek64(m_fd, m_enum_w, SEEK_SET);

	/* д���� */
	write(m_fd, m_buf, length);

	m_enum_w += length;

	return 0;
}

/*
 * Function: EnumClose
 * Desc: �ر��ļ�ö��
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CHashF::EnumClose()
{
	return 0;
}
