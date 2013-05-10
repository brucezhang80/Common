/*
 * @File: bhashf.cpp
 * @Desc: impliment file of Binary-Hash File
 * @Author: stevetang
 * @History:
 *      2009-04-23   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "common.h"
#include "bhashf.h"

CBaseBHashF::CBaseBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen /* = 1024 * 1024 */, int max_fcnt /* = 10 */, int mul /* = 2 */, int max_size /* = 0 */) : 
	m_openType(E_READ), m_get_bucket_id(get_bucket_id), m_key_cmp(cmp_key), m_dataInfo(buflen, max_size), m_max_fcnt(max_fcnt), m_mul(mul), m_buflen(buflen)
{
	m_dataInfoEx = new CBFileEx * [max_fcnt];
	if (m_dataInfoEx == NULL)
		return;

	for (int i=0; i<max_fcnt; i++)
	{
		m_dataInfoEx[i] = NULL;
	}

	m_buf = (char *)malloc(buflen);
	m_tmp_buf = (char *)malloc(buflen);

	memset(m_path, 0, sizeof(m_path));
	memset(m_fname, 0, sizeof(m_fname));

	memset(&m_bHashF, 0, sizeof(recBHashFInfo));
}

CBaseBHashF::~CBaseBHashF()
{
	/* �ͷ���Դ */
	Close();

	if (m_dataInfoEx)
	{
		for (int i=0; i<m_max_fcnt; i++)
		{
			if (m_dataInfoEx[i])
			{
				delete m_dataInfoEx[i];
				m_dataInfoEx[i] = NULL;
			}
		}

		delete [] m_dataInfoEx;
		m_dataInfoEx = NULL;
	}

	if (m_tmp_buf)
	{
		free(m_tmp_buf);
		m_tmp_buf = NULL;
	}

	if (m_buf)
	{
		free(m_buf);
		m_buf = NULL;
	}
}

/*
 * Function: IsExist
 * Desc: ��������Ƿ����
 * In: 
 *     const char *    path    �ļ���
 *     const char *    fname   �ļ���
 * Out: 
 *     none
 * Return code: 
 *      0                   ������
 *      1                   ����
 */
bool CBaseBHashF::IsExist(const char * path, const char * fname)
{
	char name[256];

	/* ��Ϣ�ļ��Ƿ���� */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_INFO);
	if (access(name, F_OK))
		return 0;

	/* �����ļ��Ƿ���� */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_IDX);
	if (access(name, F_OK))
		return 0;

	/* �����ļ��Ƿ���� */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_DATA);
	if (access(name, F_OK))
		return 0;

	return 1;
}

/*
 * Function: GetData
 * Desc: ��ȡ����
 * In: 
 *     const void *   key         ��ֵ
 * Out: 
 *     void *         data        ����
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::GetData(const void * buf, int num, int ilen, const void * key, char * &data)
{
	/* û������ */
	if (num == 0)
	{
		return -1;
	}

	char * ptr = (char *)buf;

	/* �����ֲ��� */
	int low = 0;
	int high = num - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * ilen;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* ��ֵ��ͬ */
		{
			/* �������� */
			data = addr;
			return mid;
		}

		if (ret > 0)
			high = mid - 1;
		else
			low = mid + 1;
	}

	data = (char *)buf + low * ilen;

	return -1;
}

/*
 * Function: WriteData
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::WriteData(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* ����Ƿ���� */
	char * ptr = m_buf; /* ������ */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
	{
		/* ������ */
		if (((info->num+1) * m_bHashF.dlen_) > (unsigned int)m_buflen)
			return -1;

		/* �������� */
		int big = (ptr - m_buf) / m_bHashF.dlen_;
		if (info->num > big)
		{
			/* ����,���� */
			memcpy(m_tmp_buf, ptr, (info->num - big) * m_bHashF.dlen_);
			memcpy(ptr+m_bHashF.dlen_, m_tmp_buf, (info->num - big) * m_bHashF.dlen_);
		}
		memcpy(ptr, data, m_bHashF.dlen_);

		len = (info->num + 1) * m_bHashF.dlen_;
		/* д���� */
		int ret = m_dataInfo.Write(site, m_buf, len);
		if (ret < 0)
		{
			/* ���³����ļ� */
			int f_id = WriteDataEx(site, m_buf, len);
			if (f_id < 0)
				return -1;

			/* �������������ļ���� */
			info->f_id = f_id;
		}

		info->num++;
	}
	else
	{
		/* �Ѵ��� */
		if (force == 0)
			return 1;

		memcpy(ptr, data, m_bHashF.dlen_);

		/* д���� */
		int ret = m_dataInfo.Write(site, m_buf, len);
		if (ret < 0)
			return -1;
	}

	return 0;
}

/*
 * Function: WriteDataEx
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::WriteDataEx(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	int f_id = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[f_id];
	if (bfile == NULL)
		return -1;

	/* ��ȡ������ */
	int len = m_buflen;
	if (info->num && bfile->Read(site, m_buf, len))
		return -1;

	/* �������� */
	char * ptr = m_buf; /* ������ */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
	{
		/* ������ */
		if (((info->num+1) * m_bHashF.dlen_) > (unsigned int)m_buflen)
			return -1;

		/* �������� */
		int big = (ptr - m_buf) / m_bHashF.dlen_;
		if (info->num > big)
		{
			memcpy(m_tmp_buf, ptr, (info->num - big) * m_bHashF.dlen_);
			memcpy(ptr+m_bHashF.dlen_, m_tmp_buf, (info->num - big) * m_bHashF.dlen_);
		}
		memcpy(ptr, data, m_bHashF.dlen_);

		len = (info->num + 1) * m_bHashF.dlen_;

		/* д���� */
		int ret = bfile->Write(site, m_buf, len);
		if (ret < 0)
		{
			/* ���³����ļ� */
			int f_newid = WriteDataEx(site, m_buf, len);
			if (f_newid < 0)
				return -1;

			/* �������������ļ���� */
			info->f_id = f_newid;
		}

		info->num++;
	}
	else
	{
		/* �Ѵ��� */
		if (force == 0)
			return 1;

		memcpy(ptr, data, m_bHashF.dlen_);

		/* д���� */
		int ret = bfile->Write(site, m_buf, len);
		if (ret < 0)
			return -1;
	}

	return 0;
}

/*
 * Function: WriteDataEx
 * Desc: д����
 * In: 
 *     int             site     Ͱ
 *     const void *    data     ����
 *     int             len      ���ݳ���
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *     >0                   �ļ����
 */
int CBaseBHashF::WriteDataEx(int site, const void * data, int len)
{
	/* ���³����ļ� */
	int f_id = len / m_bHashF.ilen_;
	if (f_id >= m_max_fcnt || f_id <= 0)
		return -1;

	if (m_dataInfoEx[f_id - 1] == NULL)
	{
		m_dataInfoEx[f_id - 1] = new CBFileEx();
		if (m_dataInfoEx[f_id - 1] == NULL)
			return -1;

		/* ���ļ� */
		char name[256];
		int ilen = f_id * m_mul * m_bHashF.ilen_;

		sprintf(name, "%s/%s%s.%d", m_path, m_fname, BHASHF_SUFFIX_DATA, f_id);
		if (m_dataInfoEx[f_id - 1]->Open(name, m_bHashF.isize_, ilen, E_WRITE))
			return -1;
	}

	CBFileEx * bfile = m_dataInfoEx[f_id - 1];
	if (bfile == NULL)
		return -1;

	/* д���� */
	int ret = bfile->Write(site, data, len);
	if (ret < 0)
		return -1;

	return f_id;
}

/*
 * Function: ReadData
 * Desc: ������
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::ReadData(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* �������� */
	char * ptr = m_buf;
	if (GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr) < 0)
		return -1;

	/* ȷ�����ݳ��� */
	if ((unsigned int)datalen < m_bHashF.dlen_)
	{
		datalen = 0;
		return 0;
	}

	/* �������� */
	memcpy(data, ptr, m_bHashF.dlen_);

	return 0;
}

/*
 * Function: ReadDataEx
 * Desc: ������
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::ReadDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* ȷ�������ļ��Ƿ���� */
	int no = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[no];
	if (bfile == NULL)
		return -1;

	/* ��ȡ������ */
	int len = m_buflen;
	if (bfile->Read(site, m_buf, len))
		return -1;

	/* �������� */
	char * ptr = m_buf;
	if (GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr) < 0)
		return -1;

	/* ȷ�����ݳ��� */
	if ((unsigned int)datalen < m_bHashF.dlen_)
	{
		datalen = 0;
		return 0;
	}

	/* �������� */
	memcpy(data, ptr, m_bHashF.dlen_);

	return 0;
}

/*
 * Function: DelData
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::DelData(int site, recBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* ����Ƿ���� */
	char * ptr = m_buf; /* ������ */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
		/* ������ */
		return 1;

	/* �Ѵ��� */
	if (data && datalen && *((unsigned int *)datalen) >= m_bHashF.dlen_)
	{
		memcpy(data, ptr, m_bHashF.dlen_);
		*datalen = m_bHashF.dlen_;
	}
	else if (datalen)
	{
		*datalen = 0;
	}

	/* �������� */
	memcpy(m_tmp_buf, ptr + m_bHashF.dlen_, (info->num - n - 1) * m_bHashF.dlen_);
	memcpy(ptr, m_tmp_buf, (info->num - n - 1) * m_bHashF.dlen_);

	/* ��պ���ڵ� */
	ptr += (info->num - n - 1) * m_bHashF.dlen_;
	memset(ptr, 0, m_bHashF.dlen_);

	info->num--;

	/* д���� */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

	return 0;
}

/*
 * Function: DelDataEx
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::DelDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	int f_id = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[f_id];
	if (bfile == NULL)
		return -1;

	/* ��ȡ������ */
	int len = m_buflen;
	if (info->num && bfile->Read(site, m_buf, len))
		return -1;

	/* �������� */
	char * ptr = m_buf; /* ������ */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
		/* ������ */
		return 1;

	/* �Ѵ��� */
	if (data && datalen && *((unsigned int *)datalen) >= m_bHashF.dlen_)
	{
		memcpy(data, ptr, m_bHashF.dlen_);
		*datalen = m_bHashF.dlen_;
	}
	else if (datalen)
	{
		*datalen = 0;
	}

	/* �������� */
	memcpy(m_tmp_buf, ptr + m_bHashF.dlen_, (info->num - n - 1) * m_bHashF.dlen_);
	memcpy(ptr, m_tmp_buf, (info->num - n - 1) * m_bHashF.dlen_);

	/* ��պ���ڵ� */
	ptr += (info->num - n - 1) * m_bHashF.dlen_;
	memset(ptr, 0, m_bHashF.dlen_);

	info->num--;

	/* д���� */
	int ret = bfile->Write(site, m_buf, len);
	if (ret < 0)
		return -1;

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
int CBaseBHashF::Open(const char * path, const char * fname, int bucket /* = 7 */, int ilen /* = 1024 */, int klen /* = 0 */, int dlen /* = 0 */, enuBFileType openType /* = E_READ */)
{
	char name[256];

	/* ȷ���ļ��Ƿ���� */
	if (!IsExist(path, fname))
	{
		/* �ļ������� */
		if (openType == E_READ)
			return -1;

		/* ������Ϣ�ļ� */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_INFO);

		sprintf(m_bHashF.name_, "%s", fname);
		m_bHashF.isize_ = bucket;
		m_bHashF.ilen_ = ilen;
		m_bHashF.klen_ = klen;
		m_bHashF.dlen_ = dlen;

		int fd = open(name, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		write(fd, &m_bHashF, sizeof(recBHashFInfo));

		close(fd);
	
		/* �������ļ� */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_IDX);
		if (m_idxInfo.Open(name, bucket, sizeof(recBHashFIdxInfo), openType))
			return -1;

		/* �������ļ� */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_DATA);
		if (m_dataInfo.Open(name, bucket, ilen, openType))
			return -1;
	}
	else
	{
		/* ����Ϣ�ļ� */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_INFO);

		int fd = open(name, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		read(fd, &m_bHashF, sizeof(recBHashFInfo));

		close(fd);
	
		/* �������ļ� */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_IDX);
		if (m_idxInfo.Open(name, bucket, sizeof(recBHashFIdxInfo), openType))
			return -1;

		/* �������ļ� */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_DATA);
		if (m_dataInfo.Open(name, bucket, ilen, openType))
			return -1;

		/* ����չ�ļ� */
		DIR * dir = opendir(path);
		if (dir == NULL)
			return -1;

		while (1)
		{
			struct dirent d;
			struct dirent *r = NULL;

			/* ��ȡĿ¼�ļ���Ϣ */
			if (readdir_r(dir, &d, &r))
				break;
			if (r == NULL)
				break;

			char tmp[100];

			sprintf(tmp, "%s%s.", fname, BHASHF_SUFFIX_DATA);
			char * p = strstr(d.d_name, tmp);
			if (p == NULL)
				continue;

			p += strlen(tmp);

			int f_no = atoi(p);
			if (f_no <= 0)
				/* �Ƿ������ļ� */
				continue;

			m_dataInfoEx[f_no - 1] = new CBFileEx();
			if (m_dataInfoEx[f_no - 1] == NULL)
				return -1;

			sprintf(name, "%s/%s%s.%d", path, fname, BHASHF_SUFFIX_DATA, f_no);
			int len = f_no * m_mul * m_bHashF.ilen_; /* ��Ԫ���� */
			if (m_dataInfoEx[f_no - 1]->Open(name, bucket, len, openType))
				return -1;
		}

		closedir(dir);
	}

	strcpy(m_path, path);
	strcpy(m_fname, fname);

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
int CBaseBHashF::Close()
{
	/* �ر������ļ� */
	m_idxInfo.Close();

	/* �ر������ļ� */
	m_dataInfo.Close();

	/* �ر���չ�����ļ� */
	for (int i=0; i<m_max_fcnt; i++)
	{
		if (m_dataInfoEx[i])
		{
			delete m_dataInfoEx[i]; /* ��ر��ļ� */
			m_dataInfoEx[i] = NULL;
		}
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
int CBaseBHashF::SetChunk(int start, int end)
{
	return m_dataInfo.SetChunk(start, end);
}

/*
 * Function: Write
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::Write(const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	int site = m_get_bucket_id(key) % m_bHashF.isize_;
	recBHashFIdxInfo * info = (recBHashFIdxInfo *)m_idxInfo.Read(site);

	if (info->f_id == 0)
	{
		return WriteData(site, info, key, data, datalen, force);
	}
	else
	{
		return WriteDataEx(site, info, key, data, datalen, force);
	}
}

/*
 * Function: Delete
 * Desc: ɾ��
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::Delete(const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	int site = m_get_bucket_id(key) % m_bHashF.isize_;
	recBHashFIdxInfo * info = (recBHashFIdxInfo *)m_idxInfo.Read(site);

	if (info->f_id == 0)
	{
		return DelData(site, info, key, data, datalen);
	}
	else
	{
		return DelDataEx(site, info, key, data, datalen);
	}
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
int CBaseBHashF::Read(const void * key, void * data, int& datalen)
{
	int site = m_get_bucket_id(key) % m_bHashF.isize_;
	recBHashFIdxInfo * info = (recBHashFIdxInfo *)m_idxInfo.Read(site);

	if (info->f_id == 0)
	{
		return ReadData(site, info, key, data, datalen);
	}
	else
	{
		return ReadDataEx(site, info, key, data, datalen);
	}
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
int CBaseBHashF::Flush()
{
	return m_dataInfo.Flush();
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
int CBaseBHashF::Attach(int maxsize, char * data)
{
	return m_dataInfo.Attach(maxsize, data);
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
int CBaseBHashF::Detach()
{
	return m_dataInfo.Detach();
}

/*
 * Function: EnumIdx
 * Desc: ö���������ݲ����д���
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::EnumIdx(const char * path, const char * fname, pBHashFEnumIdx func)
{
	char name[256];

	/* �������ļ� */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_IDX);
	if (access(name, F_OK))
		return -1;

	/* ��ȡ�ļ�������Ϣ */
	recMBFile bFile;

	int fd = open64(name, O_RDONLY);
	if (fd == -1)
		return -1;

	read(fd, &bFile, sizeof(recMBFile));

	/* ��ȡ������Ϣ */
	unsigned int site = 0;
	int ret = 0;
	while (1)
	{
		int len = (m_buflen / sizeof(recBHashFIdxInfo)) * sizeof(recBHashFIdxInfo);
		int n = read(fd, m_buf, len);
		if (n <= 0)
			break;

		for (int i=0; i<n/(int)sizeof(recBHashFIdxInfo); i++)
		{
			recBHashFIdxInfo * info = (recBHashFIdxInfo *)(m_buf + i * sizeof(recBHashFIdxInfo));

			/* �ص����� */
			ret = func(site, (const recBHashFIdxInfo *)info);
			if (ret != 0)
				break;
			site++;
		}

		if (ret != 0)
			break;
	}

	close(fd);

	return 0;
}

/*
 * Function: EnumData
 * Desc: ö�����ݲ����д���
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::EnumData(const char * path, const char * fname, pBHashFEnumData func)
{
	char name[256];

	/* �������ļ� */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_DATA);
	if (access(name, F_OK))
		return -1;

	/* ��ȡ�ļ�������Ϣ */
	recBFile b;

	int fd = open64(name, O_RDWR);
	if (fd == -1)
		return -1;

	read(fd, &b, sizeof(recBFile));

	/* ӳ���ļ�ͷ��Ϣ */
	int blen = ((b.isize_ % 8) > 0) ? (b.isize_ /8 + 1) : (b.isize_ /8);
	recBFile * bFile = (recBFile *)mmap(NULL, sizeof(recBFile) + blen, PROT_READ, MAP_SHARED, fd, 0);
	if (bFile == (recBFile *)-1)
	{
		close(fd);
		return -1;
	}

	unsigned int site = 0;
	int ret = 0;

	unsigned long long addr = lseek64(fd, bFile->addr_, SEEK_SET);
	while (1)
	{
		int len = (m_buflen / bFile->ilen_) * bFile->ilen_;
		int n = read(fd, m_buf, len);
		if (n <= 0)
			break;

		/* �Ƿ��и�����Ҫ��д���� */
		unsigned char update = 0;

		for (int i=0; i<n/(int)bFile->ilen_; i++)
		{
			unsigned char bit = 0;

			BFILE_GET_VALID(bFile->binfo_, site, bit);

			if (bit == 0x01)
			{
				/* �ص����� */
				char * p = m_buf + i * bFile->ilen_;
				unsigned char tmp = 0;
				ret = func(site, p, bFile->ilen_, &tmp);

				/* ȷ���Ƿ��и��� */
				if (update == 0 && tmp)
					update = 1;
			}

			if (ret != 0)
				break;

			site++;
		}

		if (ret != 0)
			break;

		if (update)
		{
			/* ��д���� */
			lseek64(fd, addr, SEEK_SET);
			write(fd, m_buf, n / bFile->ilen_ * bFile->ilen_);
		}

		/* �µ�ַ */
		addr += (n / bFile->ilen_ * bFile->ilen_);
	}

	close(fd);

	/* �ͷ�ӳ������� */
	munmap(bFile, sizeof(recBFile) + blen);

	return 0;
}

/*
 * Function: EnumDataEx
 * Desc: ö����չ�����ļ������д���
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseBHashF::EnumDataEx(const char * path, const char * fname, pBHashFEnumDataEx func)
{
	/* ����չ�ļ� */
	DIR * dir = opendir(path);
	if (dir == NULL)
		return -1;

	while (1)
	{
		struct dirent d;
		struct dirent *r = NULL;

		/* ��ȡĿ¼�ļ���Ϣ */
		if (readdir_r(dir, &d, &r))
			break;
		if (r == NULL)
			break;

		char tmp[100];

		sprintf(tmp, "%s%s.", fname, BHASHF_SUFFIX_DATA);
		char * p = strstr(d.d_name, tmp);
		if (p == NULL)
			continue;

		p += strlen(tmp);

		int f_no = atoi(p);
		if (f_no <= 0)
			/* �Ƿ������ļ� */
			continue;

		/* �������ļ� */
		char name[512];

		sprintf(name, "%s/%s", path, d.d_name);

		recBFileEx b;

		int fd = open64(name, O_RDWR);
		if (fd == -1)
			return -1;

		read(fd, &b, sizeof(recBFile));

		/* ӳ���ļ�ͷ��Ϣ */
		int blen = sizeof(struct _tagBFileEx::_tagBInfo) * b.isize_;
		recBFileEx * bFile = (recBFileEx *)mmap(NULL, sizeof(recBFile) + blen, PROT_READ, MAP_SHARED, fd, 0);
		if (bFile == (recBFileEx *)-1)
		{
			close(fd);
			return -1;
		}

		int ret = 0;

		unsigned long long addr = lseek64(fd, bFile->addr_, SEEK_SET);
		unsigned int ichunk = bFile->ilen_ + sizeof(unsigned int);
		while (1)
		{
			int len = (m_buflen / ichunk) * ichunk;
			int n = read(fd, m_buf, len);
			if (n <= 0)
				break;

			/* �Ƿ��и�����Ҫ��д���� */
			unsigned char update = 0;

			for (int i=0; i<n/(int)ichunk; i++)
			{
				char * p = m_buf + i * ichunk;

				/* ȡsite */
				unsigned int site = 0;
				memcpy(&site, p, sizeof(unsigned int));

				/* ������ */
				p += sizeof(unsigned int);

				unsigned char bit = 0;
				unsigned int offset = 0;

				BFILEEX_GET_VALID(bFile->binfo_, site, bit, offset);

				if (bit == 0x01)
				{
					/* �ص����� */
					unsigned char tmp = 0;
					ret = func(site, p, bFile->ilen_, &tmp);

					/* ȷ���Ƿ��и��� */
					if (update == 0 && tmp)
						update = 1;
				}

				if (ret != 0)
					break;
			}

			if (ret != 0)
				break;

			if (update)
			{
				/* ��д���� */
				lseek64(fd, addr, SEEK_SET);
				write(fd, m_buf, n / ichunk * ichunk);
			}

			/* �µ�ַ */
			addr += (n / ichunk * ichunk);
		}

		close(fd);

		/* �ͷ�ӳ������� */
		munmap(bFile, sizeof(recBFile) + blen);
	}

	closedir(dir);

	return 0;
}


CBHashF::CBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen /* = 1024 * 1024 */, int max_fcnt /* = 10 */, int mul /* = 2 */, int max_size /* = 0 */) : 
CBaseBHashF(get_bucket_id, cmp_key, buflen, max_fcnt, mul, max_size)
{
}

CBHashF::~CBHashF()
{
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
int CBHashF::Write(const void * key, const void * data, unsigned char force /* = 1 */)
{
	int len = m_bHashF.dlen_;

	return CBaseBHashF::Write(key, data, len, force);
}

/*
 * Function: Delete
 * Desc: ɾ��
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBHashF::Delete(const void * key, void * data /* = NULL */)
{
	int len = m_bHashF.dlen_;

	return CBaseBHashF::Delete(key, data, &len);
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
int CBHashF::Read(const void * key, void * data)
{
	int len = m_bHashF.dlen_;

	return CBaseBHashF::Read(key, data, len);
}

CBHashFEx::CBHashFEx(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen /* = 1024 * 1024 */, int max_fcnt /* = 10 */, int mul /* = 2 */, int max_size /* = 0 */) : 
CBaseBHashF(get_bucket_id, cmp_key, buflen, max_fcnt, mul, max_size)
{
}

CBHashFEx::~CBHashFEx()
{
}

/*
 * Function: PreSort
 * Desc: ������������
 * In: 
 *     const char *    src      Դ���ݴ�
 * Out: 
 *     char *          dst      ����
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBHashFEx::PreSort(int num, const char * src, char * dst)
{
	char * s = (char *)src;
	char * d = dst;
	short offset = 0;

	for (int i=0; i<num; i++)
	{
		/* ������ֵ */
		memcpy(d, s, m_bHashF.klen_);
		d += m_bHashF.klen_;
		s += m_bHashF.klen_;

		/* ȷ�����ݳ��� */
		unsigned short len = 0;
		memcpy(&len, s, sizeof(unsigned short));
		s += sizeof(unsigned short);

		/* �������� */
		s += len;

		/* ����ƫ�� */
		memcpy(d, &offset, sizeof(short));
		d += sizeof(short);

		offset += m_bHashF.klen_ + sizeof(unsigned short) + len;
	}

	/* ����ʵ�����ݳ��� */
	return offset;
}

/*
 * Function: WriteData
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBHashFEx::WriteData(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* Ԥ���� */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* ����Ƿ���� */
	char * ptr = m_tmp_buf; /* ������ */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n >= 0 && force == 0)
		/* ���ݴ����Ҳ����� */
		return 1;

	/* ȡ��ǰ���ݵ�ƫ�� */
	short offset = 0, next = total_len;
	if (n >= 0)
	{
		/* ���ݴ��� */
		memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

		/* ȡԭ�����ݳ��� */
		int olen = 0;
		memcpy(&olen, m_buf + offset + m_bHashF.klen_, sizeof(short));
		/* ȷ��ϵͳ�����Ƿ� */
		if (m_buflen < (total_len + datalen - olen))
			return -1;

		/* ȷ����һ���ڵ��ƫ�� */
		int big = (ptr - m_tmp_buf) / dlen + 1;
		ptr += m_bHashF.klen_ + sizeof(short);
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}
	}
	else
	{
		/* ���ݲ�����;ȷ��ϵͳ���峤���Ƿ� */
		if (m_buflen < (int)(total_len + m_bHashF.klen_ + sizeof(short) + datalen))
			return -1;

		int big = (ptr - m_tmp_buf) / dlen;
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}

		offset = next;
	}

	/* ���ݺ������� */
	int remain = total_len - next;
	ptr = m_buf + next;
	memcpy(m_tmp_buf, ptr, remain);

	/* ���������� */
	ptr = m_buf + offset;
	/* ���Ƽ�ֵ */
	memcpy(ptr, key, m_bHashF.klen_);
	ptr += m_bHashF.klen_;
	/* ���ݳ��� */
	memcpy(ptr, &datalen, sizeof(short));
	ptr += sizeof(short);
	/* ���� */
	memcpy(ptr, data, datalen);
	ptr += datalen;
	/* �������� */
	memcpy(ptr, m_tmp_buf, remain);
	ptr += remain;

	/* д���� */
	len = ptr - m_buf;
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
	{
		/* ���³����ļ� */
		int f_id = CBaseBHashF::WriteDataEx(site, m_buf, len);
		if (f_id < 0)
			return -1;

		/* �������������ļ���� */
		info->f_id = f_id;
	}

	/* ȷ���Ƿ�Ϊ���� */
	if (n < 0)
	{
		info->num++;
	}

	return 0;
}

/*
 * Function: WriteDataEx
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBHashFEx::WriteDataEx(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* ȡ��Ӧ�ļ� */
	int f_id = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[f_id];
	if (bfile == NULL)
		return -1;

	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (info->num && bfile->Read(site, m_buf, len))
		return -1;

	/* Ԥ���� */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* ����Ƿ���� */
	char * ptr = m_tmp_buf; /* ������ */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n >= 0 && force == 0)
		/* ���ݴ����Ҳ����� */
		return 1;

	/* ȡ��ǰ���ݵ�ƫ�� */
	short offset = 0, next = total_len;
	if (n >= 0)
	{
		/* ���ݴ��� */
		memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

		/* ȡԭ�����ݳ��� */
		int olen = 0;
		memcpy(&olen, m_buf + offset + m_bHashF.klen_, sizeof(short));
		/* ȷ��ϵͳ�����Ƿ� */
		if (m_buflen < (total_len + datalen - olen))
			return -1;

		/* ȷ����һ���ڵ��ƫ�� */
		int big = (ptr - m_tmp_buf) / dlen + 1;
		ptr += m_bHashF.klen_ + sizeof(short);
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}
	}
	else
	{
		/* ���ݲ�����;ȷ��ϵͳ���峤���Ƿ� */
		if (m_buflen < (int)(total_len + m_bHashF.klen_ + sizeof(short) + datalen))
			return -1;

		int big = (ptr - m_tmp_buf) / dlen;
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}

		offset = next;
	}

	/* ���ݺ������� */
	int remain = total_len - next;
	ptr = m_buf + next;
	memcpy(m_tmp_buf, ptr, remain);

	/* ���������� */
	ptr = m_buf + offset;
	/* ���Ƽ�ֵ */
	memcpy(ptr, key, m_bHashF.klen_);
	ptr += m_bHashF.klen_;
	/* ���ݳ��� */
	memcpy(ptr, &datalen, sizeof(short));
	ptr += sizeof(short);
	/* ���� */
	memcpy(ptr, data, datalen);
	ptr += datalen;
	/* �������� */
	memcpy(ptr, m_tmp_buf, remain);
	ptr += remain;

	/* д���� */
	len = ptr - m_buf;
	int ret = bfile->Write(site, m_buf, len);
	if (ret < 0)
	{
		/* ���³����ļ� */
		int f_id = CBaseBHashF::WriteDataEx(site, m_buf, len);
		if (f_id < 0)
			return -1;

		/* �������������ļ���� */
		info->f_id = f_id;
	}

	/* ȷ���Ƿ�Ϊ���� */
	if (n < 0)
	{
		info->num++;
	}

	return 0;
}

/*
 * Function: ReadData
 * Desc: ������
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBHashFEx::ReadData(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* Ԥ���� */
	PreSort(info->num, m_buf, m_tmp_buf);

	/* ����Ƿ���� */
	char * ptr = m_tmp_buf; /* ������ */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* ���ݲ����� */
		return -1;

	/* ���ݴ��� */
	short offset = 0;
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	ptr = m_buf + offset;

	/* ������ֵ */
	ptr += m_bHashF.klen_;

	/* ȡ���ݳ��� */
	memcpy(&len, ptr, sizeof(short));
	ptr += sizeof(short);

	if (datalen < len)
	{
		datalen = 0;
		return 0;
	}

	/* ȡ���� */
	memcpy(data, ptr, len);
	datalen = len;

	return 0;
}

/*
 * Function: ReadDataEx
 * Desc: ������
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBHashFEx::ReadDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* ȷ�������ļ��Ƿ���� */
	int no = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[no];
	if (bfile == NULL)
		return -1;

	/* ��ȡ������ */
	int len = m_buflen;
	if (bfile->Read(site, m_buf, len))
		return -1;

	/* Ԥ���� */
	PreSort(info->num, m_buf, m_tmp_buf);

	/* ����Ƿ���� */
	char * ptr = m_tmp_buf; /* ������ */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* ���ݲ����� */
		return -1;

	/* ���ݴ��� */
	short offset = 0;
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	ptr = m_buf + offset;

	/* ������ֵ */
	ptr += m_bHashF.klen_;

	/* ȡ���ݳ��� */
	memcpy(&len, ptr, sizeof(short));
	ptr += sizeof(short);

	if (datalen < len)
	{
		datalen = 0;
		return 0;
	}

	/* ȡ���� */
	memcpy(data, ptr, len);
	datalen = len;

	return 0;
}

/*
 * Function: DelData
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBHashFEx::DelData(int site, recBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* Ԥ���� */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* ����Ƿ���� */
	char * ptr = m_tmp_buf; /* ������ */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* ������ */
		return 1;

	/* �Ѵ��� */
	short offset = 0;
	/* ���ݴ��� */
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	/* ȡ���ݳ��� */
	ptr = m_buf + offset;
	/* ������ֵ */
	ptr += m_bHashF.klen_;
	/* ȡ���� */
	int olen = 0;
	memcpy(&olen, ptr, sizeof(short));
	ptr += sizeof(short);

	/* ȡ���� */
	if (data && datalen && *datalen >= olen)
	{
		memcpy(data, ptr, olen);
		*datalen = olen;
	}
	ptr += olen;

	/* ȡʣ������ */
	int remain = total_len - (ptr - m_buf);

	/* �������� */
	memcpy(m_tmp_buf, ptr, remain);

	ptr = m_buf + offset;
	memcpy(ptr, m_tmp_buf, remain);
	
	/* ��պ���ڵ� */
	ptr += remain;
	remain = len - (ptr - m_buf);
	memset(ptr, 0, remain);

	/* д���� */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

	info->num--;

	return 0;
}

/*
 * Function: DelDataEx
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBHashFEx::DelDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	int f_id = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[f_id];
	if (bfile == NULL)
		return -1;

	/* ��ȡ������ */
	int len = m_buflen;
	if (info->num && bfile->Read(site, m_buf, len))
		return -1;

	/* Ԥ���� */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* ����Ƿ���� */
	char * ptr = m_tmp_buf; /* ������ */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* ������ */
		return 1;

	/* �Ѵ��� */
	short offset = 0;
	/* ���ݴ��� */
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	/* ȡ���ݳ��� */
	ptr = m_buf + offset;
	/* ������ֵ */
	ptr += m_bHashF.klen_;
	/* ȡ���� */
	int olen = 0;
	memcpy(&olen, ptr, sizeof(short));
	ptr += sizeof(short);

	/* ȡ���� */
	if (data && datalen && *datalen >= olen)
	{
		memcpy(data, ptr, olen);
		*datalen = olen;
	}
	ptr += olen;

	/* ȡʣ������ */
	int remain = total_len - (ptr - m_buf);

	/* �������� */
	memcpy(m_tmp_buf, ptr, remain);

	ptr = m_buf + offset;
	memcpy(ptr, m_tmp_buf, remain);
	
	/* ��պ���ڵ� */
	ptr += remain;
	remain = len - (ptr - m_buf);
	memset(ptr, 0, remain);

	/* д���� */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

	info->num--;

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
int CBHashFEx::Open(const char * path, const char * fname, int bucket /* = 7 */, int ilen /* = 1024 */, int klen /* = 0 */, enuBFileType openType /* = E_READ */)
{
	return CBaseBHashF::Open(path, fname, bucket, ilen, klen, -1, openType);
}

CBaseMinBHashF::CBaseMinBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen /* = 1024 * 1024 */) : 
	m_openType(E_READ), m_get_bucket_id(get_bucket_id), m_key_cmp(cmp_key), m_dataInfo(buflen), m_buflen(buflen)
{
	m_buf = (char *)malloc(buflen);
	m_tmp_buf = (char *)malloc(buflen);

	memset(m_path, 0, sizeof(m_path));
	memset(m_fname, 0, sizeof(m_fname));

	memset(&m_bHashF, 0, sizeof(recBHashFInfo));
}

CBaseMinBHashF::~CBaseMinBHashF()
{
	/* �ͷ���Դ */
	Close();

	if (m_tmp_buf)
	{
		free(m_tmp_buf);
		m_tmp_buf = NULL;
	}

	if (m_buf)
	{
		free(m_buf);
		m_buf = NULL;
	}
}

/*
 * Function: IsExist
 * Desc: ��������Ƿ����
 * In: 
 *     const char *    path    �ļ���
 *     const char *    fname   �ļ���
 * Out: 
 *     none
 * Return code: 
 *      0                   ������
 *      1                   ����
 */
bool CBaseMinBHashF::IsExist(const char * path, const char * fname)
{
	char name[256];

	/* ��Ϣ�ļ��Ƿ���� */
	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_INFO);
	if (access(name, F_OK))
		return 0;

	/* �����ļ��Ƿ���� */
	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_IDX);
	if (access(name, F_OK))
		return 0;

	/* �����ļ��Ƿ���� */
	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_DATA);
	if (access(name, F_OK))
		return 0;

	return 1;
}

/*
 * Function: GetData
 * Desc: ��ȡ����
 * In: 
 *     const void *   key         ��ֵ
 * Out: 
 *     void *         data        ����
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseMinBHashF::GetData(const void * buf, int num, int ilen, const void * key, char * &data)
{
	/* û������ */
	if (num == 0)
	{
		return -1;
	}

	char * ptr = (char *)buf;

	/* �����ֲ��� */
	int low = 0;
	int high = num - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * ilen;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* ��ֵ��ͬ */
		{
			/* �������� */
			data = addr;
			return mid;
		}

		if (ret > 0)
			high = mid - 1;
		else
			low = mid + 1;
	}

	data = (char *)buf + low * ilen;

	return -1;
}

/*
 * Function: WriteDataEx
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseMinBHashF::WriteData(int site, recMinBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* ��ȡ������ */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* �������� */
	char * ptr = m_buf; /* ������ */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
	{
		/* ������ */
		if (((info->num+1) * m_bHashF.dlen_) > (unsigned int)m_buflen)
			return -1;

		/* �������� */
		int big = (ptr - m_buf) / m_bHashF.dlen_;
		if (info->num > big)
		{
			/* ����,���� */
			memcpy(m_tmp_buf, ptr, (info->num - big) * m_bHashF.dlen_);
			memcpy(ptr+m_bHashF.dlen_, m_tmp_buf, (info->num - big) * m_bHashF.dlen_);
		}
		memcpy(ptr, data, m_bHashF.dlen_);

		len = (info->num + 1) * m_bHashF.dlen_;
		/* д���� */
		int ret = m_dataInfo.Write(site, m_buf, len, 0);
		if (ret < 0)
		{
			/* д����ʧ�� */
			return -1;
		}

		info->num++;
	}
	else
	{
		/* �Ѵ��� */
		if (force == 0)
			return 1;

		memcpy(ptr, data, m_bHashF.dlen_);

		/* д���� */
		int ret = m_dataInfo.Write(site, m_buf, len, 0);
		if (ret < 0)
			return -1;
	}

	return 0;
}

/*
 * Function: WriteDataEx
 * Desc: д����
 * In: 
 *     int             site     Ͱ
 *     const void *    data     ����
 *     int             len      ���ݳ���
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseMinBHashF::WriteData(int site, const void * data, int len)
{
	/* д���� */
	int ret = m_dataInfo.Write(site, data, len);
	if (ret < 0)
		return -1;

	return 0;
}

/*
 * Function: ReadDataEx
 * Desc: ������
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseMinBHashF::ReadData(int site, recMinBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* �������� */
	char * ptr = m_buf;
	if (GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr) < 0)
		return -1;

	/* ȷ�����ݳ��� */
	if ((unsigned int)datalen < m_bHashF.dlen_)
	{
		datalen = 0;
		return 0;
	}

	/* �������� */
	memcpy(data, ptr, m_bHashF.dlen_);

	return 0;
}

/*
 * Function: DelData
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseMinBHashF::DelData(int site, recMinBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* ����Ƿ���� */
	char * ptr = m_buf; /* ������ */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
		/* ������ */
		return 1;

	/* �Ѵ��� */
	if (data && datalen && *((unsigned int *)datalen) >= m_bHashF.dlen_)
	{
		memcpy(data, ptr, m_bHashF.dlen_);
		*datalen = m_bHashF.dlen_;
	}
	else if (datalen)
	{
		*datalen = 0;
	}

	/* �������� */
	memcpy(m_tmp_buf, ptr + m_bHashF.dlen_, (info->num - n - 1) * m_bHashF.dlen_);
	memcpy(ptr, m_tmp_buf, (info->num - n - 1) * m_bHashF.dlen_);

	/* ��պ���ڵ� */
	ptr += (info->num - n - 1) * m_bHashF.dlen_;
	memset(ptr, 0, m_bHashF.dlen_);

	info->num--;

	/* д���� */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

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
int CBaseMinBHashF::Open(const char * path, const char * fname, int bucket /* = 7 */, int ilen /* = 1024 */, int klen /* = 0 */, int dlen /* = 0 */, enuBFileType openType /* = E_READ */)
{
	char name[256];

	/* ȷ���ļ��Ƿ���� */
	if (!IsExist(path, fname))
	{
		/* �ļ������� */
		if (openType == E_READ)
			return -1;

		/* ������Ϣ�ļ� */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_INFO);

		sprintf(m_bHashF.name_, "%s", fname);
		m_bHashF.isize_ = bucket;
		m_bHashF.ilen_ = ilen;
		m_bHashF.klen_ = klen;
		m_bHashF.dlen_ = dlen;

		int fd = open(name, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		write(fd, &m_bHashF, sizeof(recBHashFInfo));

		close(fd);
	
		/* �������ļ� */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_IDX);
		if (m_idxInfo.Open(name, bucket, sizeof(recMinBHashFIdxInfo), openType))
			return -1;

		/* �������ļ� */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_DATA);
		if (m_dataInfo.Open(name, bucket, ilen, openType))
			return -1;
	}
	else
	{
		/* ����Ϣ�ļ� */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_INFO);

		int fd = open(name, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		read(fd, &m_bHashF, sizeof(recBHashFInfo));

		close(fd);
	
		/* �������ļ� */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_IDX);
		if (m_idxInfo.Open(name, bucket, sizeof(recMinBHashFIdxInfo), openType))
			return -1;

		/* �������ļ� */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_DATA);
		if (m_dataInfo.Open(name, bucket, ilen, openType))
			return -1;
	}

	strcpy(m_path, path);
	strcpy(m_fname, fname);

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
int CBaseMinBHashF::Close()
{
	/* �ر������ļ� */
	m_idxInfo.Close();

	/* �ر������ļ� */
	m_dataInfo.Close();

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
int CBaseMinBHashF::Write(const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	int site = m_get_bucket_id(key) % m_bHashF.isize_;
	recMinBHashFIdxInfo * info = (recMinBHashFIdxInfo *)m_idxInfo.Read(site);

	return WriteData(site, info, key, data, datalen, force);
}

/*
 * Function: Delete
 * Desc: ɾ��
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseMinBHashF::Delete(const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	int site = m_get_bucket_id(key) % m_bHashF.isize_;
	recMinBHashFIdxInfo * info = (recMinBHashFIdxInfo *)m_idxInfo.Read(site);

	if (DelData(site, info, key, data, datalen) < 0)
		return -1;

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
int CBaseMinBHashF::Read(const void * key, void * data, int& datalen)
{
	int site = m_get_bucket_id(key) % m_bHashF.isize_;
	recMinBHashFIdxInfo * info = (recMinBHashFIdxInfo *)m_idxInfo.Read(site);

	return ReadData(site, info, key, data, datalen);
}

/*
 * Function: EnumIdx
 * Desc: ö���������ݲ����д���
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseMinBHashF::EnumIdx(const char * path, const char * fname, pBHashFEnumIdx func)
{
	char name[256];

	/* �������ļ� */
	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_IDX);
	if (access(name, F_OK))
		return -1;

	/* ��ȡ�ļ�������Ϣ */
	recMBFile bFile;

	int fd = open64(name, O_RDONLY);
	if (fd == -1)
		return -1;

	read(fd, &bFile, sizeof(recMBFile));

	/* ��ȡ������Ϣ */
	unsigned int site = 0;
	int ret = 0;
	while (1)
	{
		int len = (m_buflen / sizeof(recMinBHashFIdxInfo)) * sizeof(recMinBHashFIdxInfo);
		int n = read(fd, m_buf, len);
		if (n <= 0)
			break;

		for (int i=0; i<n/(int)sizeof(recMinBHashFIdxInfo); i++)
		{
			recMinBHashFIdxInfo * info = (recMinBHashFIdxInfo *)(m_buf + i * sizeof(recMinBHashFIdxInfo));

			/* �ص����� */
			ret = func(site, (const recMinBHashFIdxInfo *)info);
			if (ret != 0)
				break;
			site++;
		}

		if (ret != 0)
			break;
	}

	close(fd);

	return 0;
}

/*
 * Function: EnumDataEx
 * Desc: ö����չ�����ļ������д���
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CBaseMinBHashF::EnumData(const char * path, const char * fname, pBHashFEnumDataEx func)
{
	/* �������ļ� */
	char name[512];

	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_DATA);

	recBFileEx b;

	int fd = open64(name, O_RDWR);
	if (fd == -1)
		return -1;

	read(fd, &b, sizeof(recBFile));

	/* ӳ���ļ�ͷ��Ϣ */
	int blen = sizeof(struct _tagBFileEx::_tagBInfo) * b.isize_;
	recBFileEx * bFile = (recBFileEx *)mmap(NULL, sizeof(recBFile) + blen, PROT_READ, MAP_SHARED, fd, 0);
	if (bFile == (recBFileEx *)-1)
	{
		close(fd);
		return -1;
	}

	int ret = 0;

	unsigned long long addr = lseek64(fd, bFile->addr_, SEEK_SET);
	unsigned int ichunk = bFile->ilen_ + sizeof(unsigned int);
	while (1)
	{
		int len = (m_buflen / ichunk) * ichunk;
		int n = read(fd, m_buf, len);
		if (n <= 0)
			break;

		/* �Ƿ��и�����Ҫ��д���� */
		unsigned char update = 0;

		for (int i=0; i<n/(int)ichunk; i++)
		{
			char * p = m_buf + i * ichunk;

			/* ȡsite */
			unsigned int site = 0;
			memcpy(&site, p, sizeof(unsigned int));

			/* ������ */
			p += sizeof(unsigned int);

			unsigned char bit = 0;
			unsigned int offset = 0;

			BFILEEX_GET_VALID(bFile->binfo_, site, bit, offset);

			if (bit == 0x01)
			{
				/* �ص����� */
				unsigned char tmp = 0;
				ret = func(site, p, bFile->ilen_, &tmp);

				/* ȷ���Ƿ��и��� */
				if (update == 0 && tmp)
					update = 1;
			}

			if (ret != 0)
				break;
		}

		if (ret != 0)
			break;

		if (update)
		{
			/* ��д���� */
			lseek64(fd, addr, SEEK_SET);
			write(fd, m_buf, n / ichunk * ichunk);
		}

		/* �µ�ַ */
		addr += (n / ichunk * ichunk);
	}

	close(fd);

	/* �ͷ�ӳ������� */
	munmap(bFile, sizeof(recBFile) + blen);

	return 0;
}

CMinBHashF::CMinBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen /* = 1024 * 1024 */) : 
	CBaseMinBHashF(get_bucket_id, cmp_key, buflen)
{
}

CMinBHashF::~CMinBHashF()
{
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
int CMinBHashF::Write(const void * key, const void * data, unsigned char force /* = 1 */)
{
	int len = m_bHashF.dlen_;

	return CBaseMinBHashF::Write(key, data, len, force);
}

/*
 * Function: Delete
 * Desc: ɾ��
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMinBHashF::Delete(const void * key, void * data /* = NULL */)
{
	int len = m_bHashF.dlen_;

	return CBaseMinBHashF::Delete(key, data, &len);
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
int CMinBHashF::Read(const void * key, void * data)
{
	int len = m_bHashF.dlen_;

	return CBaseMinBHashF::Read(key, data, len);
}

CMinBHashFEx::CMinBHashFEx(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen /* = 1024 * 1024 */) : 
	CBaseMinBHashF(get_bucket_id, cmp_key, buflen)
{
}

CMinBHashFEx::~CMinBHashFEx()
{
}

/*
 * Function: PreSort
 * Desc: ������������
 * In: 
 *     const char *    src      Դ���ݴ�
 * Out: 
 *     char *          dst      ����
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMinBHashFEx::PreSort(int num, const char * src, char * dst)
{
	char * s = (char *)src;
	char * d = dst;
	short offset = 0;

	for (int i=0; i<num; i++)
	{
		/* ������ֵ */
		memcpy(d, s, m_bHashF.klen_);
		d += m_bHashF.klen_;
		s += m_bHashF.klen_;

		/* ȷ�����ݳ��� */
		unsigned short len = 0;
		memcpy(&len, s, sizeof(unsigned short));
		s += sizeof(unsigned short);

		/* �������� */
		s += len;

		/* ����ƫ�� */
		memcpy(d, &offset, sizeof(short));
		d += sizeof(short);

		offset += m_bHashF.klen_ + sizeof(unsigned short) + len;
	}

	/* ����ʵ�����ݳ��� */
	return offset;
}

/*
 * Function: WriteData
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMinBHashFEx::WriteData(int site, recMinBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* Ԥ���� */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* ����Ƿ���� */
	char * ptr = m_tmp_buf; /* ������ */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n >= 0 && force == 0)
		/* ���ݴ����Ҳ����� */
		return 1;

	/* ȡ��ǰ���ݵ�ƫ�� */
	short offset = 0, next = total_len;
	if (n >= 0)
	{
		/* ���ݴ��� */
		memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

		/* ȡԭ�����ݳ��� */
		int olen = 0;
		memcpy(&olen, m_buf + offset + m_bHashF.klen_, sizeof(short));
		/* ȷ��ϵͳ�����Ƿ� */
		if (m_buflen < (total_len + datalen - olen))
			return -1;

		/* ȷ����һ���ڵ��ƫ�� */
		int big = (ptr - m_tmp_buf) / dlen + 1;
		ptr += m_bHashF.klen_ + sizeof(short);
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}
	}
	else
	{
		/* ���ݲ�����;ȷ��ϵͳ���峤���Ƿ� */
		if (m_buflen < (int)(total_len + m_bHashF.klen_ + sizeof(short) + datalen))
			return -1;

		int big = (ptr - m_tmp_buf) / dlen;
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}

		offset = next;
	}

	/* ���ݺ������� */
	int remain = total_len - next;
	ptr = m_buf + next;
	memcpy(m_tmp_buf, ptr, remain);

	/* ���������� */
	ptr = m_buf + offset;
	/* ���Ƽ�ֵ */
	memcpy(ptr, key, m_bHashF.klen_);
	ptr += m_bHashF.klen_;
	/* ���ݳ��� */
	memcpy(ptr, &datalen, sizeof(short));
	ptr += sizeof(short);
	/* ���� */
	memcpy(ptr, data, datalen);
	ptr += datalen;
	/* �������� */
	memcpy(ptr, m_tmp_buf, remain);
	ptr += remain;

	/* д���� */
	len = ptr - m_buf;
	int ret = m_dataInfo.Write(site, m_buf, len, 0);
	if (ret < 0)
	{
		return -1;
	}

	/* ȷ���Ƿ�Ϊ���� */
	if (n < 0)
	{
		info->num++;
	}

	return 0;
}

/*
 * Function: ReadData
 * Desc: ������
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMinBHashFEx::ReadData(int site, recMinBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* Ԥ���� */
	PreSort(info->num, m_buf, m_tmp_buf);

	/* ����Ƿ���� */
	char * ptr = m_tmp_buf; /* ������ */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* ���ݲ����� */
		return -1;

	/* ���ݴ��� */
	short offset = 0;
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	ptr = m_buf + offset;

	/* ������ֵ */
	ptr += m_bHashF.klen_;

	/* ȡ���ݳ��� */
	memcpy(&len, ptr, sizeof(short));
	ptr += sizeof(short);

	if (datalen < len)
	{
		datalen = 0;
		return 0;
	}

	/* ȡ���� */
	memcpy(data, ptr, len);
	datalen = len;

	return 0;
}

/*
 * Function: DelData
 * Desc: д����
 * In: 
 *     const void *    key      ��ֵ
 *     const void *    data     ����
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMinBHashFEx::DelData(int site, recMinBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	/* ��ʼ���ļ� */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* Ԥ���� */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* ����Ƿ���� */
	char * ptr = m_tmp_buf; /* ������ */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* ������ */
		return 1;

	/* �Ѵ��� */
	short offset = 0;
	/* ���ݴ��� */
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	/* ȡ���ݳ��� */
	ptr = m_buf + offset;
	/* ������ֵ */
	ptr += m_bHashF.klen_;
	/* ȡ���� */
	int olen = 0;
	memcpy(&olen, ptr, sizeof(short));
	ptr += sizeof(short);

	/* ȡ���� */
	if (data && datalen && *datalen >= olen)
	{
		memcpy(data, ptr, olen);
		*datalen = olen;
	}
	ptr += olen;

	/* ȡʣ������ */
	int remain = total_len - (ptr - m_buf);

	/* �������� */
	memcpy(m_tmp_buf, ptr, remain);

	ptr = m_buf + offset;
	memcpy(ptr, m_tmp_buf, remain);
	
	/* ��պ���ڵ� */
	ptr += remain;
	remain = len - (ptr - m_buf);
	memset(ptr, 0, remain);

	/* д���� */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

	info->num--;

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
int CMinBHashFEx::Open(const char * path, const char * fname, int bucket /* = 7 */, int ilen /* = 1024 */, int klen /* = 0 */, enuBFileType openType /* = E_READ */)
{
	return CBaseMinBHashF::Open(path, fname, bucket, ilen, klen, -1, openType);
}
