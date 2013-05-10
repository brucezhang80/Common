/*
 * @File: common.h
 * @Desc: head file of Common
 * @Author: stevetang
 * @History:
 *      2007-11-26   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _COMMON_H
#define _COMMON_H

/*
 * Function: FlatStr
 * Desc: ����ַ�������β�ո�ͻس�����
 * In: 
 *     char *   buf   �ַ���
 * Out: 
 *     char *   buf   �ַ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int FlatStr(char * buf);

/*
 * Function: TrimStr
 * Desc: �����ַ���,ȥ���س�
 * In: 
 *     char *   buf   �ַ���
 * Out: 
 *     char *   buf   �ַ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int TrimStr(char * buf);

/*
 * Function: CheckHalfChinese
 * Desc: ȥ������İ������
 * In: 
 *     char *   buf   �ַ���
 * Out: 
 *     char *   buf   �ַ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CheckHalfChinese(char * buf);

/*
 * Function: GetLocalIP
 * Desc: ȡ����IP��ַ
 * In: 
 *     char *   ip   �ַ���
 * Out: 
 *     char *   ip   �ַ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int GetLocalIP(char * ip);

// ����if��ȡ������Ӧ��ip
int GetIpByIf(char *ifname, char *pszip); 

//����Ŀ¼
int CreateDirectory(const char * path);

//��ȡ�ļ���
int GetFileName(const char * path, char * fname);

//��ȡ·����
int GetPath(const char * fname, char * path);

//��ȡ�ļ�������չ��
int GetFileExt(const char * path, char * fname, char * ext);

#endif
