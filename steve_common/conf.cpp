/*
 * @File: conf.cpp
 * @Desc: impliment file of ini file
 * @Author: stevetang
 * @History:
 *      2008-10-07   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "common.h"
#include "conf.h"

CConf::CConf(const char * lpConfFile) : m_nFileLines(0), m_FileContainer(NULL)
{
	/* ���������ļ������ݳ��� */
	m_nFileLines = CountFileLine(lpConfFile);
	if (m_nFileLines <= 0)
		return;

	/* �������ݿռ� */
	m_FileContainer = (char **)malloc(m_nFileLines * sizeof(char *));
	if (m_FileContainer == NULL)
		return;
	bzero(m_FileContainer, m_nFileLines * sizeof(char *));

	/* ����������Ϣ */
	LoadConfInfo(lpConfFile);
}

CConf::~CConf()
{
	if (m_FileContainer)
	{
		for (int i=0; i<m_nFileLines; i++)
		{
			if (m_FileContainer[i])
			{
				free(m_FileContainer[i]);
				m_FileContainer[i] = NULL;
			}
		}

		free(m_FileContainer);
		m_FileContainer = NULL;
	}
}

/*
 * Function: CountFileLine
 * Desc: ��������
 * In: 
 *     const char *   lpConfFile  �ļ���
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *     >=0                  ����
 */
int CConf::CountFileLine(const char * lpConfFile)
{
	int cnt = 0;

	FILE * fp = fopen(lpConfFile, "r");
	if (fp == NULL)
		return -1;

	while (!feof(fp))
	{
		char line[1024];

		/* �ļ��ж�ȡ���� */
		if (fgets(line, sizeof(line)-1, fp) == NULL)
			continue;

		/* ������ҿո񡢻س������� */
		FlatStr(line);

		if (strlen(line) <= 0 || line[0] == '#')
			continue;

		cnt++;
	}

	fclose(fp);

	return cnt;
}

/*
 * Function: LoadConfInfo
 * Desc: ���������ļ�����
 * In: 
 *     const char *   lpConfFile  �ļ���
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *     0                    �ɹ�
 */
int CConf::LoadConfInfo(const char * lpConfFile)
{
	FILE * fp = fopen(lpConfFile, "r");
	if (fp == NULL)
		return -1;

	int n = 0;

	while (!feof(fp))
	{
		char line[1024];

		/* �ļ��ж�ȡ���� */
		if (fgets(line, sizeof(line)-1, fp) == NULL)
			continue;

		/* ������ҿո񡢻س������� */
		FlatStr(line);

		if (strlen(line) <= 0 || line[0] == '#')
			continue;

		/* �������ݻ����� */
		m_FileContainer[n] = strdup(line);

		n++;

		if (n >= m_nFileLines)
			break;
	}

	fclose(fp);

	return 0;
}

/*
 * Function: SearchLine
 * Desc: ȡ����Ӧ����������
 * In: 
 *     const char *   section  ����
 *     const char *   name     ������
 * Out: 
 *     char *         value    ֵ
 * Return code: 
 *     -1                   ����
 *     0                    �ɹ�
 */
int CConf::SearchLine(const char * section, const char * name, char * value)
{
	char line[1024];
	unsigned char bFindSection = 0;

	sprintf(line, "[%s]", section);
	for (int i=0; i<m_nFileLines; i++)
	{
		/* �ж�section */
		if (strcmp(m_FileContainer[i], line) == 0)
		{
			bFindSection = 1;
			continue;
		}

		/* ���Σ�û�ҵ����� */
		if (bFindSection && m_FileContainer[i][0] == '[')
		{
			break;
		}

		if (bFindSection == 0)
			continue;

		/* ȡ������ */
		char * str = strchr(m_FileContainer[i], '=');
		if (str == NULL)
			continue;

		char tmp[1024];

		strncpy(tmp, m_FileContainer[i], str-m_FileContainer[i]);
		tmp[str-m_FileContainer[i]] = 0x00;

		/* ����ո� */
		FlatStr(tmp);

		if (strcmp(tmp, name) == 0)
		{
			/* ȡֵ */
			strcpy(tmp, str+1);

			/* ����ո� */
			FlatStr(tmp);

			strcpy(value, tmp);

			return i;
		}
	}

	return -1;
}

/*
 * Function: GetStr
 * Desc: ȡ�ַ���
 * In: 
 *     const char *   section  ����
 *     const char *   name     ������
 *     int            len      ���峤��
 *     const char *   def_val  Ĭ��ֵ
 * Out: 
 *     char *         value    ֵ
 * Return code: 
 *     -1                   ����
 *     0                    �ɹ�
 */
int CConf::GetStr(const char * section, const char * name, char * value, int len, const char * def_val /* = "" */)
{
	char tmp[1024];

	/* ����Ĭ��ֵ */
	strcpy(value, def_val);

	/* ������Ӧ�� */
	if (SearchLine(section, name, tmp) <= 0)
		return -1;

	/* ���峤�Ȳ��� */
	if (strlen(tmp) > (unsigned int)len)
		return -2;

	strcpy(value, tmp);

	return 0;
}

/*
 * Function: GetInt
 * Desc: ȡ����ֵ
 * In: 
 *     const char *   section  ����
 *     const char *   name     ������
 *     const int      def_val  Ĭ��ֵ
 * Out: 
 *     char *         value    ֵ
 * Return code: 
 *     -1                   ����
 *     0                    �ɹ�
 */
int CConf::GetInt(const char * section, const char * name, int * value, const int def_val /* = 0 */)
{
	char tmp[1024];

	/* ����Ĭ��ֵ */
	*value = def_val;

	/* ������Ӧ�� */
	if (SearchLine(section, name, tmp) <= 0)
		return -1;

	*value = atoi(tmp);

	return 0;
}

/*
 * Function: getDouble
 * Desc: ȡ�����Ͳ���
 * In: 
 *     const char * section ����
 *     const char * key     ��ֵ
 *     const double def     ȱʡֵ
 * Out: 
 *     double *     value   ����
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CConf::GetDouble(const char * section, const char * name, double * value, const double def_val /* = 0.0 */)
{
	char tmp[1024];

	/* ����Ĭ��ֵ */
	*value = def_val;

	/* ������Ӧ�� */
	if (SearchLine(section, name, tmp) <= 0)
		return -1;

	*value = (double)atof(tmp);

	return 0;
}
