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
	/* 计算配置文件中数据长度 */
	m_nFileLines = CountFileLine(lpConfFile);
	if (m_nFileLines <= 0)
		return;

	/* 分配数据空间 */
	m_FileContainer = (char **)malloc(m_nFileLines * sizeof(char *));
	if (m_FileContainer == NULL)
		return;
	bzero(m_FileContainer, m_nFileLines * sizeof(char *));

	/* 加载配置信息 */
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
 * Desc: 计算行数
 * In: 
 *     const char *   lpConfFile  文件名
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *     >=0                  行数
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

		/* 文件中读取数据 */
		if (fgets(line, sizeof(line)-1, fp) == NULL)
			continue;

		/* 清除左右空格、回车、换行 */
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
 * Desc: 加载配置文件数据
 * In: 
 *     const char *   lpConfFile  文件名
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *     0                    成功
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

		/* 文件中读取数据 */
		if (fgets(line, sizeof(line)-1, fp) == NULL)
			continue;

		/* 清除左右空格、回车、换行 */
		FlatStr(line);

		if (strlen(line) <= 0 || line[0] == '#')
			continue;

		/* 放入数据缓冲中 */
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
 * Desc: 取得相应数据所在行
 * In: 
 *     const char *   section  段名
 *     const char *   name     参数名
 * Out: 
 *     char *         value    值
 * Return code: 
 *     -1                   出错
 *     0                    成功
 */
int CConf::SearchLine(const char * section, const char * name, char * value)
{
	char line[1024];
	unsigned char bFindSection = 0;

	sprintf(line, "[%s]", section);
	for (int i=0; i<m_nFileLines; i++)
	{
		/* 判断section */
		if (strcmp(m_FileContainer[i], line) == 0)
		{
			bFindSection = 1;
			continue;
		}

		/* 换段，没找到数据 */
		if (bFindSection && m_FileContainer[i][0] == '[')
		{
			break;
		}

		if (bFindSection == 0)
			continue;

		/* 取参数名 */
		char * str = strchr(m_FileContainer[i], '=');
		if (str == NULL)
			continue;

		char tmp[1024];

		strncpy(tmp, m_FileContainer[i], str-m_FileContainer[i]);
		tmp[str-m_FileContainer[i]] = 0x00;

		/* 清除空格 */
		FlatStr(tmp);

		if (strcmp(tmp, name) == 0)
		{
			/* 取值 */
			strcpy(tmp, str+1);

			/* 清除空格 */
			FlatStr(tmp);

			strcpy(value, tmp);

			return i;
		}
	}

	return -1;
}

/*
 * Function: GetStr
 * Desc: 取字符串
 * In: 
 *     const char *   section  段名
 *     const char *   name     参数名
 *     int            len      缓冲长度
 *     const char *   def_val  默认值
 * Out: 
 *     char *         value    值
 * Return code: 
 *     -1                   出错
 *     0                    成功
 */
int CConf::GetStr(const char * section, const char * name, char * value, int len, const char * def_val /* = "" */)
{
	char tmp[1024];

	/* 设置默认值 */
	strcpy(value, def_val);

	/* 查找相应行 */
	if (SearchLine(section, name, tmp) <= 0)
		return -1;

	/* 缓冲长度不够 */
	if (strlen(tmp) > (unsigned int)len)
		return -2;

	strcpy(value, tmp);

	return 0;
}

/*
 * Function: GetInt
 * Desc: 取整数值
 * In: 
 *     const char *   section  段名
 *     const char *   name     参数名
 *     const int      def_val  默认值
 * Out: 
 *     char *         value    值
 * Return code: 
 *     -1                   出错
 *     0                    成功
 */
int CConf::GetInt(const char * section, const char * name, int * value, const int def_val /* = 0 */)
{
	char tmp[1024];

	/* 设置默认值 */
	*value = def_val;

	/* 查找相应行 */
	if (SearchLine(section, name, tmp) <= 0)
		return -1;

	*value = atoi(tmp);

	return 0;
}

/*
 * Function: getDouble
 * Desc: 取浮点型参数
 * In: 
 *     const char * section 段名
 *     const char * key     键值
 *     const double def     缺省值
 * Out: 
 *     double *     value   参数
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CConf::GetDouble(const char * section, const char * name, double * value, const double def_val /* = 0.0 */)
{
	char tmp[1024];

	/* 设置默认值 */
	*value = def_val;

	/* 查找相应行 */
	if (SearchLine(section, name, tmp) <= 0)
		return -1;

	*value = (double)atof(tmp);

	return 0;
}
