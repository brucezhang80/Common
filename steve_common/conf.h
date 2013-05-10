/*
 * @File: conf.h
 * @Desc: head file of ini file
 * @Author: stevetang
 * @History:
 *      2008-10-07   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _CONF_H
#define _CONF_H

class CConf
{
	int m_nFileLines; /* 文件有效数据行数 */
	char ** m_FileContainer; /* 文件有效数据 */

public:
	CConf(const char * lpConfFile);
	~CConf();

private:
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
	int CountFileLine(const char * lpConfFile);

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
	int LoadConfInfo(const char * lpConfFile);

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
	int SearchLine(const char * section, const char * name, char * value);

public:
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
	int GetStr(const char * section, const char * name, char * value, int len, const char * def_val = "");

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
	int GetInt(const char * section, const char * name, int * value, const int def_val = 0);

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
    int GetDouble(const char * section, const char * name, double * value, const double def_val = 0.0);
};

#endif
