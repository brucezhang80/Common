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
 * Desc: 清除字符串的首尾空格和回车换行
 * In: 
 *     char *   buf   字符串
 * Out: 
 *     char *   buf   字符串
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int FlatStr(char * buf);

/*
 * Function: TrimStr
 * Desc: 整理字符串,去掉回车
 * In: 
 *     char *   buf   字符串
 * Out: 
 *     char *   buf   字符串
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int TrimStr(char * buf);

/*
 * Function: CheckHalfChinese
 * Desc: 去掉多余的半个汉字
 * In: 
 *     char *   buf   字符串
 * Out: 
 *     char *   buf   字符串
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CheckHalfChinese(char * buf);

/*
 * Function: GetLocalIP
 * Desc: 取本地IP地址
 * In: 
 *     char *   ip   字符串
 * Out: 
 *     char *   ip   字符串
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int GetLocalIP(char * ip);

// 根据if获取本机对应的ip
int GetIpByIf(char *ifname, char *pszip); 

//创建目录
int CreateDirectory(const char * path);

//提取文件名
int GetFileName(const char * path, char * fname);

//提取路径名
int GetPath(const char * fname, char * path);

//提取文件名和扩展名
int GetFileExt(const char * path, char * fname, char * ext);

#endif
