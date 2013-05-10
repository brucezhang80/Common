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
	/* ���������ļ��ռ� */
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

		/* ������ȡ���� */
		m_InInfo[i]->in = CFileSeqRead::Create(m_buflen);
		if (m_InInfo[i]->in == NULL)
			return;

		/* ������ݵĻ��� */
		m_InInfo[i]->data = (char *)malloc(m_datalen);
		if (m_InInfo[i]->data == NULL)
			return;
	}

	/* ��������ļ��ռ� */
	m_OutInfo = (recMergeOutInfo *)malloc(sizeof(recMergeOutInfo));
	if (m_OutInfo == NULL)
		return;
	memset(m_OutInfo, 0, sizeof(recMergeOutInfo));

	/* ����������� */
	m_OutInfo->out = CFileSeqWrite::Create(m_buflen);
	if (m_OutInfo->out == NULL)
		return;

	/* ������ݵĻ��� */
	m_OutInfo->data = (char *)malloc(m_datalen);
	if (m_OutInfo->data == NULL)
		return;
}

CMulMerge::~CMulMerge()
{
	if (m_OutInfo)
	{
		/* �ͷ�д���� */
		if (m_OutInfo->out)
		{
			CFileSeqWrite::Destroy(m_OutInfo->out);
		}

		/* �ͷ����ݿռ� */
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

			/* �ͷŶ����� */
			if (inInfo->in)
			{
				CFileSeqRead::Destroy(inInfo->in);
			}

			/* �ͷ����ݿռ� */
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
 * Desc: ��ȡ��������
 * In: 
 *     int   n              ��Ӧ��������
 * Out: 
 *     none
 * Return code: 
 *     NULL                 ����
 *     ��NULL               �ɹ�
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
 * Desc: ��ȡ�������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     NULL                 ����
 *     ��NULL               �ɹ�
 */
char * CMulMerge::GetOutData()
{
	return m_OutInfo->data;
}

/*
 * Function: SetOutData
 * Desc: �����������
 * In: 
 *     const char *   data  ��������
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
 * Desc: ���������ļ�
 * In: 
 *     const char *    fname   �ļ���
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMulMerge::AddInFile(const char * fname, int len)
{
	if (m_cur >= m_num || m_InInfo == NULL || len > m_datalen)
		return -1;

	recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[m_cur];
	if (inInfo == NULL)
		return -1;

	/* ���ļ� */
	if (!inInfo->in->Open(fname))
		return -1;

	/* ����������Ч */
	inInfo->valid = 0;

	/* �������ݳ��� */
	inInfo->len = len;

	m_cur++;

	return 0;
}

/*
 * Function: SetOutFile
 * Desc: ��������ļ���
 * In: 
 *     const char *    fname   �ļ���
 *     unsigned char   isnew   �Ƿ�����µĿռ䣬���������ṹ��ͬʱ��
 *     int             len     ����ṹ��С
 *     int             size    ����ļ������С
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMulMerge::SetOutFile(const char * fname, unsigned char isnew, int len)
{
	if (m_OutInfo == NULL || len > m_datalen)
		return -1;

	/* ���ļ� */
	if (!m_OutInfo->out->Open(fname))
		return -1;

	m_OutInfo->isnew = isnew;
	m_OutInfo->len = len;
	m_OutInfo->valid = 0;

	return 0;
}

/*
 * Function: GetMin
 * Desc: ��ȡ��С����������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *     >0                   �ɹ�
 */
int CMulMerge::GetMin()
{
	int valid = 0;
	int same = 0;
	int min = -1;
	int i = 0;

	/* ȡ��С����Ч���� */
	for (i=0; i<m_cur; i++)
	{
		recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[i];
		if (inInfo == NULL)
			return -1;

		if (!inInfo->valid) /* ��Ч���ݣ�������Ƚ� */
			continue;

		valid++; /* ��Ч����ͳ�� */

		if (min < 0)
		{
			min = i;
			same = 1;
			continue;
		}

		/* �Ƚ����� */
		int ret = Compare(min, i);
		if (ret > 0) /* С�� */
		{
			min = i;
			same = 1;
		}
		else if (ret == 0) /* ��ͬ */
		{
			same++;
		}
	}

	if (min < 0)
		return -1; /* �鲢����� */

	if (same == valid && valid > 1) /* ���еĶ�Ϊ��С����Ч�����ݳ���1�����������ɨ����һ�ڵ� */
	{
		recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[min];
		if (inInfo == NULL)
			return -1;
		inInfo->valid = 0; /* ����Ϊ��Ч */

		m_OutInfo->valid = 0; /* �����Ч */

		printf("ok[%d-%d][%llu].\n", same, valid, *((unsigned long long *)GetInData(min)));
		exit(0);
	}
	else
	{
		/* ��������������Ϊ��Ч���� */
		for (i=min; i<m_cur; i++)
		{
			recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[i];
			if (inInfo == NULL)
				return -1;

			if (!inInfo->valid)
				continue;

			/* �Ƚ����� */
			int ret = Compare(min, i);
			if (ret == 0)
			{
				inInfo->valid = 0; /* ����Ϊ��Ч */
			}
		}

		/* ����������� */
		SetOut(min);
		m_OutInfo->valid = 1;
	}

	return min;
}

/*
 * Function: Merge
 * Desc: ��·�鲢
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMulMerge::Merge()
{
	if (m_InInfo == NULL || m_OutInfo == NULL)
		return -1;

	while (1)
	{
		/* ׼������ */
		for (int i=0; i<m_cur; i++)
		{
			recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[i];
			if (inInfo == NULL)
				return -1;

			if (inInfo->valid)
				continue;

			/* ��ȡ���� */
			if (inInfo->in->Read(inInfo->data, inInfo->len) <= 0)
				continue;

			inInfo->valid = 1; /* ����Ϊ��Ч���� */
		}

		/* ��ȡ��С������ */
		int min = GetMin();
		if (min < 0)
			break; /* �����ѹ鲢��� */

		/* д����ļ� */
		if (m_OutInfo->valid)
		{
			if (m_OutInfo->out->Write(m_OutInfo->data, m_OutInfo->len) <= 0)
				return -1;

			m_OutInfo->valid = 0; /* ����Ϊ��Ч���� */
		}
	}

	return 0;
}

/*
 * Function: Close
 * Desc: ��ʼ�������ļ�������ļ���Ŀ
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMulMerge::Close()
{
	/* �ر������ļ� */
	for (int i=0; i<m_cur; i++)
	{
		recMergeInInfo * inInfo = (recMergeInInfo *)m_InInfo[i];
		if (inInfo == NULL)
			return -1;

		inInfo->in->Close();
		inInfo->valid = 0;
	}

	/* �ر�����ļ� */
	m_OutInfo->out->Close();
	m_OutInfo->valid = 0;

	m_cur = 0;

	return 0;
}
