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
	int m_nFileLines; /* �ļ���Ч�������� */
	char ** m_FileContainer; /* �ļ���Ч���� */

public:
	CConf(const char * lpConfFile);
	~CConf();

private:
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
	int CountFileLine(const char * lpConfFile);

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
	int LoadConfInfo(const char * lpConfFile);

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
	int SearchLine(const char * section, const char * name, char * value);

public:
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
	int GetStr(const char * section, const char * name, char * value, int len, const char * def_val = "");

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
	int GetInt(const char * section, const char * name, int * value, const int def_val = 0);

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
    int GetDouble(const char * section, const char * name, double * value, const double def_val = 0.0);
};

#endif
