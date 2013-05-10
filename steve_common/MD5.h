////////////////////////////////////////////////////////////////////////////////
// 版权信息：(C) 2003, Jerry.CS.HUST.China
// 文件名：  MD5.h
// 作者：    Jerry
// 功能描述：对数据进行MD5加密
// 版本：    V1.1.2003.02.25
// 修改历史：
////////////////////////////////////////////////////////////////////////////////

#ifndef _MD5_H_
#define _MD5_H_

typedef unsigned char *POINTER;
typedef unsigned short int UINT2;
typedef unsigned long int UINT4;

//CDESCrypt类的定义
class MD5
{
public:

typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

public:
	MD5(){}
	virtual ~MD5(){}

	// 供外部使用的哈希函数：生成16bytes的hash值
	static void encode(unsigned char* p_pszDest, const char* p_pszSrc);

	// 供外部使用的哈希函数：生成long long的字符串形式hash值
    static void Hash64(const char* sInputUrl, char* sOutputDocID);	

	// 供外部使用的哈希函数：生成long long的hash值
    static void Hash64(const char* sInputUrl, long long& ddwDocID);

private:
	// MD5 初始化. 开始一个MD5操作, 写一个新的上下文
	static void MD5Init (MD5_CTX *);

	// MD5块的更新操作，继续MD5报文摘要操作，处理另外的报文块，并更新上下文 
	static void MD5Update(MD5_CTX *, unsigned char *, unsigned int);

	// MD5结束操作，写报文摘要并清空上下文
	static void MD5Final (unsigned char [16], MD5_CTX *);

	// MD5的主要变换过程
	static void MD5Transform (UINT4 [4], unsigned char [64]);

	// 把输入的UINT4数据编码后输出成unsigned char
	static void innerEncode (unsigned char *, UINT4 *, unsigned int);

	// 把输入的unsigned char数据解码后输出成UINT4
	static void innerDecode (UINT4 *, unsigned char *, unsigned int);

	// 两个内存块数据之间复制,注意：这里不能用strcpy
	static void MD5_memcpy (POINTER, POINTER, unsigned int);

	// 把一块内存全部初始化为某一个值
	static void MD5_memset (POINTER, int, unsigned int);
	
};

#endif
