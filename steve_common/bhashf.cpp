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
	/* 释放资源 */
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
bool CBaseBHashF::IsExist(const char * path, const char * fname)
{
	char name[256];

	/* 信息文件是否存在 */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_INFO);
	if (access(name, F_OK))
		return 0;

	/* 索引文件是否存在 */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_IDX);
	if (access(name, F_OK))
		return 0;

	/* 数据文件是否存在 */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_DATA);
	if (access(name, F_OK))
		return 0;

	return 1;
}

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
int CBaseBHashF::GetData(const void * buf, int num, int ilen, const void * key, char * &data)
{
	/* 没有数据 */
	if (num == 0)
	{
		return -1;
	}

	char * ptr = (char *)buf;

	/* 做二分查找 */
	int low = 0;
	int high = num - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * ilen;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* 键值相同 */
		{
			/* 设置数据 */
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
int CBaseBHashF::WriteData(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 检查是否存在 */
	char * ptr = m_buf; /* 数据区 */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
	{
		/* 不存在 */
		if (((info->num+1) * m_bHashF.dlen_) > (unsigned int)m_buflen)
			return -1;

		/* 复制数据 */
		int big = (ptr - m_buf) / m_bHashF.dlen_;
		if (info->num > big)
		{
			/* 排序,插入 */
			memcpy(m_tmp_buf, ptr, (info->num - big) * m_bHashF.dlen_);
			memcpy(ptr+m_bHashF.dlen_, m_tmp_buf, (info->num - big) * m_bHashF.dlen_);
		}
		memcpy(ptr, data, m_bHashF.dlen_);

		len = (info->num + 1) * m_bHashF.dlen_;
		/* 写数据 */
		int ret = m_dataInfo.Write(site, m_buf, len);
		if (ret < 0)
		{
			/* 打开新长度文件 */
			int f_id = WriteDataEx(site, m_buf, len);
			if (f_id < 0)
				return -1;

			/* 设置数据所在文件编号 */
			info->f_id = f_id;
		}

		info->num++;
	}
	else
	{
		/* 已存在 */
		if (force == 0)
			return 1;

		memcpy(ptr, data, m_bHashF.dlen_);

		/* 写数据 */
		int ret = m_dataInfo.Write(site, m_buf, len);
		if (ret < 0)
			return -1;
	}

	return 0;
}

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
int CBaseBHashF::WriteDataEx(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	int f_id = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[f_id];
	if (bfile == NULL)
		return -1;

	/* 读取段数据 */
	int len = m_buflen;
	if (info->num && bfile->Read(site, m_buf, len))
		return -1;

	/* 查找数据 */
	char * ptr = m_buf; /* 数据区 */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
	{
		/* 不存在 */
		if (((info->num+1) * m_bHashF.dlen_) > (unsigned int)m_buflen)
			return -1;

		/* 复制数据 */
		int big = (ptr - m_buf) / m_bHashF.dlen_;
		if (info->num > big)
		{
			memcpy(m_tmp_buf, ptr, (info->num - big) * m_bHashF.dlen_);
			memcpy(ptr+m_bHashF.dlen_, m_tmp_buf, (info->num - big) * m_bHashF.dlen_);
		}
		memcpy(ptr, data, m_bHashF.dlen_);

		len = (info->num + 1) * m_bHashF.dlen_;

		/* 写数据 */
		int ret = bfile->Write(site, m_buf, len);
		if (ret < 0)
		{
			/* 打开新长度文件 */
			int f_newid = WriteDataEx(site, m_buf, len);
			if (f_newid < 0)
				return -1;

			/* 设置数据所在文件编号 */
			info->f_id = f_newid;
		}

		info->num++;
	}
	else
	{
		/* 已存在 */
		if (force == 0)
			return 1;

		memcpy(ptr, data, m_bHashF.dlen_);

		/* 写数据 */
		int ret = bfile->Write(site, m_buf, len);
		if (ret < 0)
			return -1;
	}

	return 0;
}

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
int CBaseBHashF::WriteDataEx(int site, const void * data, int len)
{
	/* 打开新长度文件 */
	int f_id = len / m_bHashF.ilen_;
	if (f_id >= m_max_fcnt || f_id <= 0)
		return -1;

	if (m_dataInfoEx[f_id - 1] == NULL)
	{
		m_dataInfoEx[f_id - 1] = new CBFileEx();
		if (m_dataInfoEx[f_id - 1] == NULL)
			return -1;

		/* 打开文件 */
		char name[256];
		int ilen = f_id * m_mul * m_bHashF.ilen_;

		sprintf(name, "%s/%s%s.%d", m_path, m_fname, BHASHF_SUFFIX_DATA, f_id);
		if (m_dataInfoEx[f_id - 1]->Open(name, m_bHashF.isize_, ilen, E_WRITE))
			return -1;
	}

	CBFileEx * bfile = m_dataInfoEx[f_id - 1];
	if (bfile == NULL)
		return -1;

	/* 写数据 */
	int ret = bfile->Write(site, data, len);
	if (ret < 0)
		return -1;

	return f_id;
}

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
int CBaseBHashF::ReadData(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 查找数据 */
	char * ptr = m_buf;
	if (GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr) < 0)
		return -1;

	/* 确认数据长度 */
	if ((unsigned int)datalen < m_bHashF.dlen_)
	{
		datalen = 0;
		return 0;
	}

	/* 复制数据 */
	memcpy(data, ptr, m_bHashF.dlen_);

	return 0;
}

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
int CBaseBHashF::ReadDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* 确认数据文件是否存在 */
	int no = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[no];
	if (bfile == NULL)
		return -1;

	/* 读取段数据 */
	int len = m_buflen;
	if (bfile->Read(site, m_buf, len))
		return -1;

	/* 查找数据 */
	char * ptr = m_buf;
	if (GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr) < 0)
		return -1;

	/* 确认数据长度 */
	if ((unsigned int)datalen < m_bHashF.dlen_)
	{
		datalen = 0;
		return 0;
	}

	/* 复制数据 */
	memcpy(data, ptr, m_bHashF.dlen_);

	return 0;
}

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
int CBaseBHashF::DelData(int site, recBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 检查是否存在 */
	char * ptr = m_buf; /* 数据区 */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
		/* 不存在 */
		return 1;

	/* 已存在 */
	if (data && datalen && *((unsigned int *)datalen) >= m_bHashF.dlen_)
	{
		memcpy(data, ptr, m_bHashF.dlen_);
		*datalen = m_bHashF.dlen_;
	}
	else if (datalen)
	{
		*datalen = 0;
	}

	/* 复制数据 */
	memcpy(m_tmp_buf, ptr + m_bHashF.dlen_, (info->num - n - 1) * m_bHashF.dlen_);
	memcpy(ptr, m_tmp_buf, (info->num - n - 1) * m_bHashF.dlen_);

	/* 清空后面节点 */
	ptr += (info->num - n - 1) * m_bHashF.dlen_;
	memset(ptr, 0, m_bHashF.dlen_);

	info->num--;

	/* 写数据 */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

	return 0;
}

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
int CBaseBHashF::DelDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	int f_id = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[f_id];
	if (bfile == NULL)
		return -1;

	/* 读取段数据 */
	int len = m_buflen;
	if (info->num && bfile->Read(site, m_buf, len))
		return -1;

	/* 查找数据 */
	char * ptr = m_buf; /* 数据区 */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
		/* 不存在 */
		return 1;

	/* 已存在 */
	if (data && datalen && *((unsigned int *)datalen) >= m_bHashF.dlen_)
	{
		memcpy(data, ptr, m_bHashF.dlen_);
		*datalen = m_bHashF.dlen_;
	}
	else if (datalen)
	{
		*datalen = 0;
	}

	/* 复制数据 */
	memcpy(m_tmp_buf, ptr + m_bHashF.dlen_, (info->num - n - 1) * m_bHashF.dlen_);
	memcpy(ptr, m_tmp_buf, (info->num - n - 1) * m_bHashF.dlen_);

	/* 清空后面节点 */
	ptr += (info->num - n - 1) * m_bHashF.dlen_;
	memset(ptr, 0, m_bHashF.dlen_);

	info->num--;

	/* 写数据 */
	int ret = bfile->Write(site, m_buf, len);
	if (ret < 0)
		return -1;

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
int CBaseBHashF::Open(const char * path, const char * fname, int bucket /* = 7 */, int ilen /* = 1024 */, int klen /* = 0 */, int dlen /* = 0 */, enuBFileType openType /* = E_READ */)
{
	char name[256];

	/* 确认文件是否存在 */
	if (!IsExist(path, fname))
	{
		/* 文件不存在 */
		if (openType == E_READ)
			return -1;

		/* 创建信息文件 */
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
	
		/* 打开索引文件 */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_IDX);
		if (m_idxInfo.Open(name, bucket, sizeof(recBHashFIdxInfo), openType))
			return -1;

		/* 打开数据文件 */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_DATA);
		if (m_dataInfo.Open(name, bucket, ilen, openType))
			return -1;
	}
	else
	{
		/* 打开信息文件 */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_INFO);

		int fd = open(name, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		read(fd, &m_bHashF, sizeof(recBHashFInfo));

		close(fd);
	
		/* 打开索引文件 */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_IDX);
		if (m_idxInfo.Open(name, bucket, sizeof(recBHashFIdxInfo), openType))
			return -1;

		/* 打开数据文件 */
		sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_DATA);
		if (m_dataInfo.Open(name, bucket, ilen, openType))
			return -1;

		/* 打开扩展文件 */
		DIR * dir = opendir(path);
		if (dir == NULL)
			return -1;

		while (1)
		{
			struct dirent d;
			struct dirent *r = NULL;

			/* 读取目录文件信息 */
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
				/* 非法数据文件 */
				continue;

			m_dataInfoEx[f_no - 1] = new CBFileEx();
			if (m_dataInfoEx[f_no - 1] == NULL)
				return -1;

			sprintf(name, "%s/%s%s.%d", path, fname, BHASHF_SUFFIX_DATA, f_no);
			int len = f_no * m_mul * m_bHashF.ilen_; /* 单元长度 */
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
 * Desc: 关闭文件
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CBaseBHashF::Close()
{
	/* 关闭索引文件 */
	m_idxInfo.Close();

	/* 关闭数据文件 */
	m_dataInfo.Close();

	/* 关闭扩展数据文件 */
	for (int i=0; i<m_max_fcnt; i++)
	{
		if (m_dataInfoEx[i])
		{
			delete m_dataInfoEx[i]; /* 会关闭文件 */
			m_dataInfoEx[i] = NULL;
		}
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
int CBaseBHashF::SetChunk(int start, int end)
{
	return m_dataInfo.SetChunk(start, end);
}

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
 * Desc: 临时缓冲数据写回实际数据区
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CBaseBHashF::Flush()
{
	return m_dataInfo.Flush();
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
int CBaseBHashF::Attach(int maxsize, char * data)
{
	return m_dataInfo.Attach(maxsize, data);
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
int CBaseBHashF::Detach()
{
	return m_dataInfo.Detach();
}

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
int CBaseBHashF::EnumIdx(const char * path, const char * fname, pBHashFEnumIdx func)
{
	char name[256];

	/* 打开索引文件 */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_IDX);
	if (access(name, F_OK))
		return -1;

	/* 读取文件基本信息 */
	recMBFile bFile;

	int fd = open64(name, O_RDONLY);
	if (fd == -1)
		return -1;

	read(fd, &bFile, sizeof(recMBFile));

	/* 读取数据信息 */
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

			/* 回调处理 */
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
 * Desc: 枚举数据并进行处理
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CBaseBHashF::EnumData(const char * path, const char * fname, pBHashFEnumData func)
{
	char name[256];

	/* 打开索引文件 */
	sprintf(name, "%s/%s%s", path, fname, BHASHF_SUFFIX_DATA);
	if (access(name, F_OK))
		return -1;

	/* 读取文件基本信息 */
	recBFile b;

	int fd = open64(name, O_RDWR);
	if (fd == -1)
		return -1;

	read(fd, &b, sizeof(recBFile));

	/* 映射文件头信息 */
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

		/* 是否有更新需要回写数据 */
		unsigned char update = 0;

		for (int i=0; i<n/(int)bFile->ilen_; i++)
		{
			unsigned char bit = 0;

			BFILE_GET_VALID(bFile->binfo_, site, bit);

			if (bit == 0x01)
			{
				/* 回调处理 */
				char * p = m_buf + i * bFile->ilen_;
				unsigned char tmp = 0;
				ret = func(site, p, bFile->ilen_, &tmp);

				/* 确认是否有更新 */
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
			/* 回写数据 */
			lseek64(fd, addr, SEEK_SET);
			write(fd, m_buf, n / bFile->ilen_ * bFile->ilen_);
		}

		/* 新地址 */
		addr += (n / bFile->ilen_ * bFile->ilen_);
	}

	close(fd);

	/* 释放映射的区间 */
	munmap(bFile, sizeof(recBFile) + blen);

	return 0;
}

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
int CBaseBHashF::EnumDataEx(const char * path, const char * fname, pBHashFEnumDataEx func)
{
	/* 打开扩展文件 */
	DIR * dir = opendir(path);
	if (dir == NULL)
		return -1;

	while (1)
	{
		struct dirent d;
		struct dirent *r = NULL;

		/* 读取目录文件信息 */
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
			/* 非法数据文件 */
			continue;

		/* 处理单个文件 */
		char name[512];

		sprintf(name, "%s/%s", path, d.d_name);

		recBFileEx b;

		int fd = open64(name, O_RDWR);
		if (fd == -1)
			return -1;

		read(fd, &b, sizeof(recBFile));

		/* 映射文件头信息 */
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

			/* 是否有更新需要回写数据 */
			unsigned char update = 0;

			for (int i=0; i<n/(int)ichunk; i++)
			{
				char * p = m_buf + i * ichunk;

				/* 取site */
				unsigned int site = 0;
				memcpy(&site, p, sizeof(unsigned int));

				/* 数据区 */
				p += sizeof(unsigned int);

				unsigned char bit = 0;
				unsigned int offset = 0;

				BFILEEX_GET_VALID(bFile->binfo_, site, bit, offset);

				if (bit == 0x01)
				{
					/* 回调处理 */
					unsigned char tmp = 0;
					ret = func(site, p, bFile->ilen_, &tmp);

					/* 确认是否有更新 */
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
				/* 回写数据 */
				lseek64(fd, addr, SEEK_SET);
				write(fd, m_buf, n / ichunk * ichunk);
			}

			/* 新地址 */
			addr += (n / ichunk * ichunk);
		}

		close(fd);

		/* 释放映射的区间 */
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
int CBHashF::Write(const void * key, const void * data, unsigned char force /* = 1 */)
{
	int len = m_bHashF.dlen_;

	return CBaseBHashF::Write(key, data, len, force);
}

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
int CBHashF::Delete(const void * key, void * data /* = NULL */)
{
	int len = m_bHashF.dlen_;

	return CBaseBHashF::Delete(key, data, &len);
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
 * Desc: 生成排序数据
 * In: 
 *     const char *    src      源数据串
 * Out: 
 *     char *          dst      数据
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CBHashFEx::PreSort(int num, const char * src, char * dst)
{
	char * s = (char *)src;
	char * d = dst;
	short offset = 0;

	for (int i=0; i<num; i++)
	{
		/* 拷贝键值 */
		memcpy(d, s, m_bHashF.klen_);
		d += m_bHashF.klen_;
		s += m_bHashF.klen_;

		/* 确认数据长度 */
		unsigned short len = 0;
		memcpy(&len, s, sizeof(unsigned short));
		s += sizeof(unsigned short);

		/* 跳过数据 */
		s += len;

		/* 数据偏移 */
		memcpy(d, &offset, sizeof(short));
		d += sizeof(short);

		offset += m_bHashF.klen_ + sizeof(unsigned short) + len;
	}

	/* 返回实际数据长度 */
	return offset;
}

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
int CBHashFEx::WriteData(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 预查找 */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* 检查是否存在 */
	char * ptr = m_tmp_buf; /* 数据区 */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n >= 0 && force == 0)
		/* 数据存在且不更新 */
		return 1;

	/* 取当前数据的偏移 */
	short offset = 0, next = total_len;
	if (n >= 0)
	{
		/* 数据存在 */
		memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

		/* 取原有数据长度 */
		int olen = 0;
		memcpy(&olen, m_buf + offset + m_bHashF.klen_, sizeof(short));
		/* 确认系统缓冲是否够 */
		if (m_buflen < (total_len + datalen - olen))
			return -1;

		/* 确定下一个节点的偏移 */
		int big = (ptr - m_tmp_buf) / dlen + 1;
		ptr += m_bHashF.klen_ + sizeof(short);
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}
	}
	else
	{
		/* 数据不存在;确认系统缓冲长度是否够 */
		if (m_buflen < (int)(total_len + m_bHashF.klen_ + sizeof(short) + datalen))
			return -1;

		int big = (ptr - m_tmp_buf) / dlen;
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}

		offset = next;
	}

	/* 备份后续数据 */
	int remain = total_len - next;
	ptr = m_buf + next;
	memcpy(m_tmp_buf, ptr, remain);

	/* 复制新数据 */
	ptr = m_buf + offset;
	/* 复制键值 */
	memcpy(ptr, key, m_bHashF.klen_);
	ptr += m_bHashF.klen_;
	/* 数据长度 */
	memcpy(ptr, &datalen, sizeof(short));
	ptr += sizeof(short);
	/* 数据 */
	memcpy(ptr, data, datalen);
	ptr += datalen;
	/* 后续数据 */
	memcpy(ptr, m_tmp_buf, remain);
	ptr += remain;

	/* 写数据 */
	len = ptr - m_buf;
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
	{
		/* 打开新长度文件 */
		int f_id = CBaseBHashF::WriteDataEx(site, m_buf, len);
		if (f_id < 0)
			return -1;

		/* 设置数据所在文件编号 */
		info->f_id = f_id;
	}

	/* 确认是否为新增 */
	if (n < 0)
	{
		info->num++;
	}

	return 0;
}

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
int CBHashFEx::WriteDataEx(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* 取对应文件 */
	int f_id = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[f_id];
	if (bfile == NULL)
		return -1;

	/* 初始段文件 */
	int len = m_buflen;
	if (info->num && bfile->Read(site, m_buf, len))
		return -1;

	/* 预查找 */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* 检查是否存在 */
	char * ptr = m_tmp_buf; /* 数据区 */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n >= 0 && force == 0)
		/* 数据存在且不更新 */
		return 1;

	/* 取当前数据的偏移 */
	short offset = 0, next = total_len;
	if (n >= 0)
	{
		/* 数据存在 */
		memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

		/* 取原有数据长度 */
		int olen = 0;
		memcpy(&olen, m_buf + offset + m_bHashF.klen_, sizeof(short));
		/* 确认系统缓冲是否够 */
		if (m_buflen < (total_len + datalen - olen))
			return -1;

		/* 确定下一个节点的偏移 */
		int big = (ptr - m_tmp_buf) / dlen + 1;
		ptr += m_bHashF.klen_ + sizeof(short);
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}
	}
	else
	{
		/* 数据不存在;确认系统缓冲长度是否够 */
		if (m_buflen < (int)(total_len + m_bHashF.klen_ + sizeof(short) + datalen))
			return -1;

		int big = (ptr - m_tmp_buf) / dlen;
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}

		offset = next;
	}

	/* 备份后续数据 */
	int remain = total_len - next;
	ptr = m_buf + next;
	memcpy(m_tmp_buf, ptr, remain);

	/* 复制新数据 */
	ptr = m_buf + offset;
	/* 复制键值 */
	memcpy(ptr, key, m_bHashF.klen_);
	ptr += m_bHashF.klen_;
	/* 数据长度 */
	memcpy(ptr, &datalen, sizeof(short));
	ptr += sizeof(short);
	/* 数据 */
	memcpy(ptr, data, datalen);
	ptr += datalen;
	/* 后续数据 */
	memcpy(ptr, m_tmp_buf, remain);
	ptr += remain;

	/* 写数据 */
	len = ptr - m_buf;
	int ret = bfile->Write(site, m_buf, len);
	if (ret < 0)
	{
		/* 打开新长度文件 */
		int f_id = CBaseBHashF::WriteDataEx(site, m_buf, len);
		if (f_id < 0)
			return -1;

		/* 设置数据所在文件编号 */
		info->f_id = f_id;
	}

	/* 确认是否为新增 */
	if (n < 0)
	{
		info->num++;
	}

	return 0;
}

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
int CBHashFEx::ReadData(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 预查找 */
	PreSort(info->num, m_buf, m_tmp_buf);

	/* 检查是否存在 */
	char * ptr = m_tmp_buf; /* 数据区 */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* 数据不存在 */
		return -1;

	/* 数据存在 */
	short offset = 0;
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	ptr = m_buf + offset;

	/* 跳过键值 */
	ptr += m_bHashF.klen_;

	/* 取数据长度 */
	memcpy(&len, ptr, sizeof(short));
	ptr += sizeof(short);

	if (datalen < len)
	{
		datalen = 0;
		return 0;
	}

	/* 取数据 */
	memcpy(data, ptr, len);
	datalen = len;

	return 0;
}

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
int CBHashFEx::ReadDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* 确认数据文件是否存在 */
	int no = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[no];
	if (bfile == NULL)
		return -1;

	/* 读取段数据 */
	int len = m_buflen;
	if (bfile->Read(site, m_buf, len))
		return -1;

	/* 预查找 */
	PreSort(info->num, m_buf, m_tmp_buf);

	/* 检查是否存在 */
	char * ptr = m_tmp_buf; /* 数据区 */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* 数据不存在 */
		return -1;

	/* 数据存在 */
	short offset = 0;
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	ptr = m_buf + offset;

	/* 跳过键值 */
	ptr += m_bHashF.klen_;

	/* 取数据长度 */
	memcpy(&len, ptr, sizeof(short));
	ptr += sizeof(short);

	if (datalen < len)
	{
		datalen = 0;
		return 0;
	}

	/* 取数据 */
	memcpy(data, ptr, len);
	datalen = len;

	return 0;
}

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
int CBHashFEx::DelData(int site, recBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 预查找 */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* 检查是否存在 */
	char * ptr = m_tmp_buf; /* 数据区 */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* 不存在 */
		return 1;

	/* 已存在 */
	short offset = 0;
	/* 数据存在 */
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	/* 取数据长度 */
	ptr = m_buf + offset;
	/* 跳过键值 */
	ptr += m_bHashF.klen_;
	/* 取长度 */
	int olen = 0;
	memcpy(&olen, ptr, sizeof(short));
	ptr += sizeof(short);

	/* 取数据 */
	if (data && datalen && *datalen >= olen)
	{
		memcpy(data, ptr, olen);
		*datalen = olen;
	}
	ptr += olen;

	/* 取剩余数据 */
	int remain = total_len - (ptr - m_buf);

	/* 复制数据 */
	memcpy(m_tmp_buf, ptr, remain);

	ptr = m_buf + offset;
	memcpy(ptr, m_tmp_buf, remain);
	
	/* 清空后面节点 */
	ptr += remain;
	remain = len - (ptr - m_buf);
	memset(ptr, 0, remain);

	/* 写数据 */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

	info->num--;

	return 0;
}

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
int CBHashFEx::DelDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	int f_id = info->f_id - 1;
	CBFileEx * bfile = m_dataInfoEx[f_id];
	if (bfile == NULL)
		return -1;

	/* 读取段数据 */
	int len = m_buflen;
	if (info->num && bfile->Read(site, m_buf, len))
		return -1;

	/* 预查找 */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* 检查是否存在 */
	char * ptr = m_tmp_buf; /* 数据区 */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* 不存在 */
		return 1;

	/* 已存在 */
	short offset = 0;
	/* 数据存在 */
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	/* 取数据长度 */
	ptr = m_buf + offset;
	/* 跳过键值 */
	ptr += m_bHashF.klen_;
	/* 取长度 */
	int olen = 0;
	memcpy(&olen, ptr, sizeof(short));
	ptr += sizeof(short);

	/* 取数据 */
	if (data && datalen && *datalen >= olen)
	{
		memcpy(data, ptr, olen);
		*datalen = olen;
	}
	ptr += olen;

	/* 取剩余数据 */
	int remain = total_len - (ptr - m_buf);

	/* 复制数据 */
	memcpy(m_tmp_buf, ptr, remain);

	ptr = m_buf + offset;
	memcpy(ptr, m_tmp_buf, remain);
	
	/* 清空后面节点 */
	ptr += remain;
	remain = len - (ptr - m_buf);
	memset(ptr, 0, remain);

	/* 写数据 */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

	info->num--;

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
	/* 释放资源 */
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
bool CBaseMinBHashF::IsExist(const char * path, const char * fname)
{
	char name[256];

	/* 信息文件是否存在 */
	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_INFO);
	if (access(name, F_OK))
		return 0;

	/* 索引文件是否存在 */
	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_IDX);
	if (access(name, F_OK))
		return 0;

	/* 数据文件是否存在 */
	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_DATA);
	if (access(name, F_OK))
		return 0;

	return 1;
}

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
int CBaseMinBHashF::GetData(const void * buf, int num, int ilen, const void * key, char * &data)
{
	/* 没有数据 */
	if (num == 0)
	{
		return -1;
	}

	char * ptr = (char *)buf;

	/* 做二分查找 */
	int low = 0;
	int high = num - 1;

	while (low <= high)
	{
		int mid = (low + high) / 2;

		char * addr = ptr + mid * ilen;

		int ret = m_key_cmp(addr, key);

		if (ret == 0) /* 键值相同 */
		{
			/* 设置数据 */
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
int CBaseMinBHashF::WriteData(int site, recMinBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* 读取段数据 */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 查找数据 */
	char * ptr = m_buf; /* 数据区 */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
	{
		/* 不存在 */
		if (((info->num+1) * m_bHashF.dlen_) > (unsigned int)m_buflen)
			return -1;

		/* 复制数据 */
		int big = (ptr - m_buf) / m_bHashF.dlen_;
		if (info->num > big)
		{
			/* 排序,插入 */
			memcpy(m_tmp_buf, ptr, (info->num - big) * m_bHashF.dlen_);
			memcpy(ptr+m_bHashF.dlen_, m_tmp_buf, (info->num - big) * m_bHashF.dlen_);
		}
		memcpy(ptr, data, m_bHashF.dlen_);

		len = (info->num + 1) * m_bHashF.dlen_;
		/* 写数据 */
		int ret = m_dataInfo.Write(site, m_buf, len, 0);
		if (ret < 0)
		{
			/* 写数据失败 */
			return -1;
		}

		info->num++;
	}
	else
	{
		/* 已存在 */
		if (force == 0)
			return 1;

		memcpy(ptr, data, m_bHashF.dlen_);

		/* 写数据 */
		int ret = m_dataInfo.Write(site, m_buf, len, 0);
		if (ret < 0)
			return -1;
	}

	return 0;
}

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
 *      0                   成功
 */
int CBaseMinBHashF::WriteData(int site, const void * data, int len)
{
	/* 写数据 */
	int ret = m_dataInfo.Write(site, data, len);
	if (ret < 0)
		return -1;

	return 0;
}

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
int CBaseMinBHashF::ReadData(int site, recMinBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 查找数据 */
	char * ptr = m_buf;
	if (GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr) < 0)
		return -1;

	/* 确认数据长度 */
	if ((unsigned int)datalen < m_bHashF.dlen_)
	{
		datalen = 0;
		return 0;
	}

	/* 复制数据 */
	memcpy(data, ptr, m_bHashF.dlen_);

	return 0;
}

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
int CBaseMinBHashF::DelData(int site, recMinBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 检查是否存在 */
	char * ptr = m_buf; /* 数据区 */
	int n = GetData(m_buf, info->num, m_bHashF.dlen_, key, ptr);
	if (n < 0)
		/* 不存在 */
		return 1;

	/* 已存在 */
	if (data && datalen && *((unsigned int *)datalen) >= m_bHashF.dlen_)
	{
		memcpy(data, ptr, m_bHashF.dlen_);
		*datalen = m_bHashF.dlen_;
	}
	else if (datalen)
	{
		*datalen = 0;
	}

	/* 复制数据 */
	memcpy(m_tmp_buf, ptr + m_bHashF.dlen_, (info->num - n - 1) * m_bHashF.dlen_);
	memcpy(ptr, m_tmp_buf, (info->num - n - 1) * m_bHashF.dlen_);

	/* 清空后面节点 */
	ptr += (info->num - n - 1) * m_bHashF.dlen_;
	memset(ptr, 0, m_bHashF.dlen_);

	info->num--;

	/* 写数据 */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

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
int CBaseMinBHashF::Open(const char * path, const char * fname, int bucket /* = 7 */, int ilen /* = 1024 */, int klen /* = 0 */, int dlen /* = 0 */, enuBFileType openType /* = E_READ */)
{
	char name[256];

	/* 确认文件是否存在 */
	if (!IsExist(path, fname))
	{
		/* 文件不存在 */
		if (openType == E_READ)
			return -1;

		/* 创建信息文件 */
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
	
		/* 打开索引文件 */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_IDX);
		if (m_idxInfo.Open(name, bucket, sizeof(recMinBHashFIdxInfo), openType))
			return -1;

		/* 打开数据文件 */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_DATA);
		if (m_dataInfo.Open(name, bucket, ilen, openType))
			return -1;
	}
	else
	{
		/* 打开信息文件 */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_INFO);

		int fd = open(name, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd == -1)
			return -1;

		read(fd, &m_bHashF, sizeof(recBHashFInfo));

		close(fd);
	
		/* 打开索引文件 */
		sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_IDX);
		if (m_idxInfo.Open(name, bucket, sizeof(recMinBHashFIdxInfo), openType))
			return -1;

		/* 打开数据文件 */
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
 * Desc: 关闭文件
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CBaseMinBHashF::Close()
{
	/* 关闭索引文件 */
	m_idxInfo.Close();

	/* 关闭数据文件 */
	m_dataInfo.Close();

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
int CBaseMinBHashF::Write(const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	int site = m_get_bucket_id(key) % m_bHashF.isize_;
	recMinBHashFIdxInfo * info = (recMinBHashFIdxInfo *)m_idxInfo.Read(site);

	return WriteData(site, info, key, data, datalen, force);
}

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
int CBaseMinBHashF::Read(const void * key, void * data, int& datalen)
{
	int site = m_get_bucket_id(key) % m_bHashF.isize_;
	recMinBHashFIdxInfo * info = (recMinBHashFIdxInfo *)m_idxInfo.Read(site);

	return ReadData(site, info, key, data, datalen);
}

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
int CBaseMinBHashF::EnumIdx(const char * path, const char * fname, pBHashFEnumIdx func)
{
	char name[256];

	/* 打开索引文件 */
	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_IDX);
	if (access(name, F_OK))
		return -1;

	/* 读取文件基本信息 */
	recMBFile bFile;

	int fd = open64(name, O_RDONLY);
	if (fd == -1)
		return -1;

	read(fd, &bFile, sizeof(recMBFile));

	/* 读取数据信息 */
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

			/* 回调处理 */
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
 * Desc: 枚举扩展数据文件并进行处理
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CBaseMinBHashF::EnumData(const char * path, const char * fname, pBHashFEnumDataEx func)
{
	/* 处理单个文件 */
	char name[512];

	sprintf(name, "%s/%s%s", path, fname, MINBHASHF_SUFFIX_DATA);

	recBFileEx b;

	int fd = open64(name, O_RDWR);
	if (fd == -1)
		return -1;

	read(fd, &b, sizeof(recBFile));

	/* 映射文件头信息 */
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

		/* 是否有更新需要回写数据 */
		unsigned char update = 0;

		for (int i=0; i<n/(int)ichunk; i++)
		{
			char * p = m_buf + i * ichunk;

			/* 取site */
			unsigned int site = 0;
			memcpy(&site, p, sizeof(unsigned int));

			/* 数据区 */
			p += sizeof(unsigned int);

			unsigned char bit = 0;
			unsigned int offset = 0;

			BFILEEX_GET_VALID(bFile->binfo_, site, bit, offset);

			if (bit == 0x01)
			{
				/* 回调处理 */
				unsigned char tmp = 0;
				ret = func(site, p, bFile->ilen_, &tmp);

				/* 确认是否有更新 */
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
			/* 回写数据 */
			lseek64(fd, addr, SEEK_SET);
			write(fd, m_buf, n / ichunk * ichunk);
		}

		/* 新地址 */
		addr += (n / ichunk * ichunk);
	}

	close(fd);

	/* 释放映射的区间 */
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
int CMinBHashF::Write(const void * key, const void * data, unsigned char force /* = 1 */)
{
	int len = m_bHashF.dlen_;

	return CBaseMinBHashF::Write(key, data, len, force);
}

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
int CMinBHashF::Delete(const void * key, void * data /* = NULL */)
{
	int len = m_bHashF.dlen_;

	return CBaseMinBHashF::Delete(key, data, &len);
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
 * Desc: 生成排序数据
 * In: 
 *     const char *    src      源数据串
 * Out: 
 *     char *          dst      数据
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CMinBHashFEx::PreSort(int num, const char * src, char * dst)
{
	char * s = (char *)src;
	char * d = dst;
	short offset = 0;

	for (int i=0; i<num; i++)
	{
		/* 拷贝键值 */
		memcpy(d, s, m_bHashF.klen_);
		d += m_bHashF.klen_;
		s += m_bHashF.klen_;

		/* 确认数据长度 */
		unsigned short len = 0;
		memcpy(&len, s, sizeof(unsigned short));
		s += sizeof(unsigned short);

		/* 跳过数据 */
		s += len;

		/* 数据偏移 */
		memcpy(d, &offset, sizeof(short));
		d += sizeof(short);

		offset += m_bHashF.klen_ + sizeof(unsigned short) + len;
	}

	/* 返回实际数据长度 */
	return offset;
}

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
int CMinBHashFEx::WriteData(int site, recMinBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force /* = 1 */)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 预查找 */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* 检查是否存在 */
	char * ptr = m_tmp_buf; /* 数据区 */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n >= 0 && force == 0)
		/* 数据存在且不更新 */
		return 1;

	/* 取当前数据的偏移 */
	short offset = 0, next = total_len;
	if (n >= 0)
	{
		/* 数据存在 */
		memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

		/* 取原有数据长度 */
		int olen = 0;
		memcpy(&olen, m_buf + offset + m_bHashF.klen_, sizeof(short));
		/* 确认系统缓冲是否够 */
		if (m_buflen < (total_len + datalen - olen))
			return -1;

		/* 确定下一个节点的偏移 */
		int big = (ptr - m_tmp_buf) / dlen + 1;
		ptr += m_bHashF.klen_ + sizeof(short);
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}
	}
	else
	{
		/* 数据不存在;确认系统缓冲长度是否够 */
		if (m_buflen < (int)(total_len + m_bHashF.klen_ + sizeof(short) + datalen))
			return -1;

		int big = (ptr - m_tmp_buf) / dlen;
		if (info->num > big)
		{
			memcpy(&next, ptr + m_bHashF.klen_, sizeof(short));
		}

		offset = next;
	}

	/* 备份后续数据 */
	int remain = total_len - next;
	ptr = m_buf + next;
	memcpy(m_tmp_buf, ptr, remain);

	/* 复制新数据 */
	ptr = m_buf + offset;
	/* 复制键值 */
	memcpy(ptr, key, m_bHashF.klen_);
	ptr += m_bHashF.klen_;
	/* 数据长度 */
	memcpy(ptr, &datalen, sizeof(short));
	ptr += sizeof(short);
	/* 数据 */
	memcpy(ptr, data, datalen);
	ptr += datalen;
	/* 后续数据 */
	memcpy(ptr, m_tmp_buf, remain);
	ptr += remain;

	/* 写数据 */
	len = ptr - m_buf;
	int ret = m_dataInfo.Write(site, m_buf, len, 0);
	if (ret < 0)
	{
		return -1;
	}

	/* 确认是否为新增 */
	if (n < 0)
	{
		info->num++;
	}

	return 0;
}

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
int CMinBHashFEx::ReadData(int site, recMinBHashFIdxInfo * info, const void * key, void * data, int& datalen)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 预查找 */
	PreSort(info->num, m_buf, m_tmp_buf);

	/* 检查是否存在 */
	char * ptr = m_tmp_buf; /* 数据区 */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* 数据不存在 */
		return -1;

	/* 数据存在 */
	short offset = 0;
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	ptr = m_buf + offset;

	/* 跳过键值 */
	ptr += m_bHashF.klen_;

	/* 取数据长度 */
	memcpy(&len, ptr, sizeof(short));
	ptr += sizeof(short);

	if (datalen < len)
	{
		datalen = 0;
		return 0;
	}

	/* 取数据 */
	memcpy(data, ptr, len);
	datalen = len;

	return 0;
}

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
int CMinBHashFEx::DelData(int site, recMinBHashFIdxInfo * info, const void * key, void * data /* = NULL */, int * datalen /* = NULL */)
{
	/* 初始段文件 */
	int len = m_buflen;
	if (info->num && m_dataInfo.Read(site, m_buf, len))
		return -1;

	/* 预查找 */
	int total_len = PreSort(info->num, m_buf, m_tmp_buf);

	/* 检查是否存在 */
	char * ptr = m_tmp_buf; /* 数据区 */
	int dlen = m_bHashF.klen_ + sizeof(short);
	int n = GetData(m_tmp_buf, info->num, dlen, key, ptr);
	if (n < 0)
		/* 不存在 */
		return 1;

	/* 已存在 */
	short offset = 0;
	/* 数据存在 */
	memcpy(&offset, ptr + m_bHashF.klen_, sizeof(short));

	/* 取数据长度 */
	ptr = m_buf + offset;
	/* 跳过键值 */
	ptr += m_bHashF.klen_;
	/* 取长度 */
	int olen = 0;
	memcpy(&olen, ptr, sizeof(short));
	ptr += sizeof(short);

	/* 取数据 */
	if (data && datalen && *datalen >= olen)
	{
		memcpy(data, ptr, olen);
		*datalen = olen;
	}
	ptr += olen;

	/* 取剩余数据 */
	int remain = total_len - (ptr - m_buf);

	/* 复制数据 */
	memcpy(m_tmp_buf, ptr, remain);

	ptr = m_buf + offset;
	memcpy(ptr, m_tmp_buf, remain);
	
	/* 清空后面节点 */
	ptr += remain;
	remain = len - (ptr - m_buf);
	memset(ptr, 0, remain);

	/* 写数据 */
	int ret = m_dataInfo.Write(site, m_buf, len);
	if (ret < 0)
		return -1;

	info->num--;

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
int CMinBHashFEx::Open(const char * path, const char * fname, int bucket /* = 7 */, int ilen /* = 1024 */, int klen /* = 0 */, enuBFileType openType /* = E_READ */)
{
	return CBaseMinBHashF::Open(path, fname, bucket, ilen, klen, -1, openType);
}
