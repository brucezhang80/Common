/*
 * @File: merge.cpp
 * @Desc: impliment file of Multi-Way Merge
 * @Author: stevetang
 * @History:
 *      2008-12-13   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "merge.h"

CMulMerge::CMulMerge(int num, int buflen, int datalen)
{
	/* 创建输入文件空间 */
	m_InInfo = (recMergeInInfo **)malloc(num * sizeof(recMergeInInfo *));
	if (m_InInfo == NULL)
		return;
	memset(m_InInfo, 0, num * sizeof(recMergeInInfo *));

	m_num = num;
	m_buflen = buflen;
	m_datalen = datalen;
	m_cur = 0;

	for (int i=0; i<m_num; i++)
	{
		m_InInfo[i] = (recMergeInInfo *)malloc(sizeof(recMergeInInfo));
		if (m_InInfo[i] == NULL)
			return;
		memset(m_InInfo[i], 0, sizeof(recMergeInInfo));

		/* 创建读取对象 */
		m_InInfo[i]->in = CFileSeqRead::Create(m_buflen);
		if (m_InInfo[i]->in == NULL)
			return;

		/* 存放数据的缓冲 */
		m_InInfo[i]->data = (char *)malloc(m_datalen);
		if (m_InInfo[i]->data == NULL)
			return;
	}

	/* 创建输出文件空间 */
	m_OutInfo = (recMergeOutInfo *)malloc(sizeof(recMergeOutInfo));
	if (m_OutInfo == NULL)
		return;
	memset(m_OutInfo, 0, sizeof(recMergeOutInfo));

	/* 创建输出对象 */
	m_OutInfo->out = CFileSeqWrite::Create(m_buflen);
	if (m_OutInfo->out == NULL)
		return;

	/* 存放数据的缓冲 */
	m_OutInfo->data = (char *)malloc(m_datalen);
	if (m_OutInfo->data == NULL)
		return;
}

CMulMerge::~CMulMerge()
{
	if (m_OutInfo)
	{
		/* 释放写对象 */
		if (m_OutInfo->out)
		{
			CFileSeqWrite::Destroy(m_OutInfo->out);
		}

		/* 释放数据空间 */
		if (m_OutInfo->isnew && m_OutInfo->data)
		{
			free(m_OutInfo->data);
			m_OutInfo->data = NULL;
		}

		free(m_OutInfo);
		m_OutInfo = NULL;
	}

	if (m_InInfo)
	{
		for (int i=0; i<m_num; i++)
		{
			recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[i];
			if (inInfo == NULL)
				continue;

			/* 释放读对象 */
			if (inInfo->in)
			{
				CFileSeqRead::Destroy(inInfo->in);
			}

			/* 释放数据空间 */
			if (inInfo->data)
			{
				free(inInfo->data);
				inInfo->data = NULL;
			}

			free(inInfo);
			m_InInfo[i] = NULL;
		}

		free(m_InInfo);
		m_InInfo = NULL;
	}
}

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
const char * CMulMerge::GetInData(int n)
{
	if (n >= m_num)
		return NULL;

	recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[n];
	if (inInfo == NULL)
		return NULL;

	return inInfo->data;
}

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
char * CMulMerge::GetOutData()
{
	return m_OutInfo->data;
}

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
int CMulMerge::SetOutData(const char * data)
{
	if (m_OutInfo->isnew && m_OutInfo->data)
	{
		memcpy(m_OutInfo->data, data, m_OutInfo->len);
	}
	else
	{
		m_OutInfo->data = (char *)data;
	}

	return 0;
}

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
int CMulMerge::AddInFile(const char * fname, int len)
{
	if (m_cur >= m_num || m_InInfo == NULL || len > m_datalen)
		return -1;

	recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[m_cur];
	if (inInfo == NULL)
		return -1;

	/* 打开文件 */
	if (!inInfo->in->Open(fname))
		return -1;

	/* 设置数据无效 */
	inInfo->valid = 0;

	/* 设置数据长度 */
	inInfo->len = len;

	m_cur++;

	return 0;
}

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
int CMulMerge::SetOutFile(const char * fname, unsigned char isnew, int len)
{
	if (m_OutInfo == NULL || len > m_datalen)
		return -1;

	/* 打开文件 */
	if (!m_OutInfo->out->Open(fname))
		return -1;

	m_OutInfo->isnew = isnew;
	m_OutInfo->len = len;
	m_OutInfo->valid = 0;

	return 0;
}

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
int CMulMerge::GetMin()
{
	int valid = 0;
	int same = 0;
	int min = -1;
	int i = 0;

	/* 取最小的有效数据 */
	for (i=0; i<m_cur; i++)
	{
		recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[i];
		if (inInfo == NULL)
			return -1;

		if (!inInfo->valid) /* 无效数据，不参与比较 */
			continue;

		valid++; /* 有效数据统计 */

		if (min < 0)
		{
			min = i;
			same = 1;
			continue;
		}

		/* 比较数据 */
		int ret = Compare(min, i);
		if (ret > 0) /* 小于 */
		{
			min = i;
			same = 1;
		}
		else if (ret == 0) /* 相同 */
		{
			same++;
		}
	}

	if (min < 0)
		return -1; /* 归并已完成 */

	if (same == valid && valid > 1) /* 所有的都为最小且有效的数据超过1个，不输出，扫描下一节点 */
	{
		recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[min];
		if (inInfo == NULL)
			return -1;
		inInfo->valid = 0; /* 设置为无效 */

		m_OutInfo->valid = 0; /* 输出无效 */

		printf("ok[%d-%d][%llu].\n", same, valid, *((unsigned long long *)GetInData(min)));
		exit(0);
	}
	else
	{
		/* 将输入数据设置为无效数据 */
		for (i=min; i<m_cur; i++)
		{
			recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[i];
			if (inInfo == NULL)
				return -1;

			if (!inInfo->valid)
				continue;

			/* 比较数据 */
			int ret = Compare(min, i);
			if (ret == 0)
			{
				inInfo->valid = 0; /* 设置为无效 */
			}
		}

		/* 设置输出数据 */
		SetOut(min);
		m_OutInfo->valid = 1;
	}

	return min;
}

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
int CMulMerge::Merge()
{
	if (m_InInfo == NULL || m_OutInfo == NULL)
		return -1;

	while (1)
	{
		/* 准备数据 */
		for (int i=0; i<m_cur; i++)
		{
			recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[i];
			if (inInfo == NULL)
				return -1;

			if (inInfo->valid)
				continue;

			/* 读取数据 */
			if (inInfo->in->Read(inInfo->data, inInfo->len) <= 0)
				continue;

			inInfo->valid = 1; /* 设置为有效数据 */
		}

		/* 获取最小的数据 */
		int min = GetMin();
		if (min < 0)
			break; /* 数据已归并完成 */

		/* 写输出文件 */
		if (m_OutInfo->valid)
		{
			if (m_OutInfo->out->Write(m_OutInfo->data, m_OutInfo->len) <= 0)
				return -1;

			m_OutInfo->valid = 0; /* 设置为无效数据 */
		}
	}

	return 0;
}

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
int CMulMerge::Close()
{
	/* 关闭输入文件 */
	for (int i=0; i<m_cur; i++)
	{
		recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[i];
		if (inInfo == NULL)
			return -1;

		inInfo->in->Close();
		inInfo->valid = 0;
	}

	/* 关闭输出文件 */
	m_OutInfo->out->Close();
	m_OutInfo->valid = 0;

	m_cur = 0;

	return 0;
}
