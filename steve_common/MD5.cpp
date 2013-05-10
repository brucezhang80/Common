/////////////////////////////////////////////////////////////////////////////////
//文件名：  MD5.cpp
//功能描述：实现了md5加密函数
//算法说明：
//			报文摘要(MD，MessageDigest）。它是将可变长度的报文M作为单向散列函数输入，然后得
//          出一个固定长度的标志H(M)。H(M)通常称为报文摘要(MD)，它主要用于下面情况： 
//			通信双方共享一个常规的密钥。发送端先将报文M输入给散列函数H，计算出H（M）即MD，
//			再用常规的密钥对MD进行加密，将加密的MD追加在报文M的后面，发送到接受端。接收端先
//			除去追加在报文M后面加密的MD，用已知的散列函数计算H(M)，再用自己拥有的密钥K对加密
//			的MD解密而得出真实的MD；比较计算出得H(M)和MD，若一致，则收到的报文M是真的。
//注释时间：7/27/2001
/////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "MD5.h"

#define S11 7    //这是md5变换时要用到的常量数据
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static unsigned char PADDING[64] = {   //这是补位时第一个要加上的数
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// F, G, H and I 是基本的MD5处理位操作函数，X，Y，Z为32位整数
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// 把x左移n位
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

//第1轮变换：以[a b c d x s ac]表示如下操作a = b + ((a + F(b,c,d) + X[k] + T[ac]) <<< s). 
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b);  }

//第2轮变换：以[a b c d x s ac]表示如下操作a=b + ((a + G(b,c,d) + X[k] + T[ac]) <<< s). 
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

//第3轮变换：以[a b c d x s ac]表示如下操作a = b + ((a + H(b,c,d) + X[k] + T[ac]) <<< s). 
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

//第4轮变换：以[a b c d x s ac]表示如下操作a = b + ((a + I(b,c,d) + X[k] + T[ac]) <<< s). 
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/**
 *功能描述： MD5 初始化. 开始一个MD5操作, 写一个新的上下文
 *入口参数： MD5_CTX *context 加密上下文
 *返回值：   无
 */
 void MD5::MD5Init (MD5_CTX *context)
{ 
	//这两个32位整数是用来保存待加密数据的长度，这里先初始化为0
	context->count[0] = context->count[1] = 0;

	//四个32位整数 (A,B,C,D) 用来计算信息摘要，初始化使用的是十六进制表示的数字
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
}

/**
 *功能描述：MD5块的更新操作，继续MD5报文摘要操作，处理另外的报文块，并更新上下文 
 *入口参数： 
 *			 MD5_CTX *context       加密上下文
 *			 unsigned char * input  待加密的数据
 *			 unsigned int inputLen  待加密的数据的长度
 *返回值：   无
 */
void MD5::MD5Update (MD5_CTX * context, unsigned char * input, unsigned int inputLen)
{
	unsigned int i, index, partLen;

	//按模64计算字节数
	index = (unsigned int)((context->count[0] >> 3) & 0x3F);

	// Update number of bits 
	if ((context->count[0] += ((UINT4)inputLen << 3)) < ((UINT4)inputLen << 3))
		context->count[1]++;
	context->count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	if (inputLen >= partLen) 
	{
		// 被64整除余下的部分先进行加密
		MD5_memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
		MD5Transform (context->state, context->buffer);

		// 以64个字节为一个单位来进行变换
		for (i = partLen; i + 63 < inputLen; i += 64)  
			MD5Transform (context->state, &input[i]);

		index = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	MD5_memcpy((POINTER)&context->buffer[index], (POINTER)&input[i], inputLen-i);
}

/**
 *功能描述： MD5结束操作，写报文摘要并清空上下文
 *入口参数： 
 *			unsigned char digest[16], 
 *			MD5_CTX * context
 *返回值：  无
 */
void MD5::MD5Final (unsigned char digest[16], MD5_CTX * context)
{
	unsigned char bits[8];
	unsigned int index, padLen;

	/* Save number of bits */
	innerEncode (bits, context->count, 8);

	/* Pad out to 56 mod 64.*/
	index = (unsigned int)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	MD5Update (context, PADDING, padLen);

	/* Append length (before padding) */
	MD5Update (context, bits, 8);

	// 保存状态到摘要
	innerEncode (digest, context->state, 16);

	// 把x所指向的内存块清0
	MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/**
 *功能描述：MD5的主要变换过程 
 *入口参数： 
 *			UINT4 state[4]           四个32位整数，用来计算信息摘要
 *			unsigned char block[64]  
 *返回值：  无
 *算法说明：512位的报文数据分成4个128位的数据块，依次被送到不同的散列函数进行4轮计算；
 *          每一轮都按32位的小数据块进行复杂的运算，计算完成后全部写入ABCD寄存器，用于下
 *          一轮的计算。等到最后计算完毕，在寄存器ABCD中的数据就是我们要求的MD5代码
 */
void MD5::MD5Transform (UINT4 state[4], unsigned char block[64])
{
	UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	//每一次，把数据原文存放在16个元素的数组X中
	innerDecode (x, block, 64);

	// 第一轮变换
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	// 第二轮变换
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	// 第三轮变换
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	// 第四轮变换
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	//把x所指向的内存块清0
	MD5_memset ((POINTER)x, 0, sizeof (x));
}

/**
 *功能描述： 把输入的UINT4数据编码后输出成unsigned char，假设长度len是4的倍数
 *入口参数： 
 *			unsigned char *output;
 *			UINT4 *input;
 *			unsigned int len 
 *返回值：   无
 */
void MD5::innerEncode (unsigned char * output, UINT4 * input,unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4) 
	{
		output[j]   = (unsigned char)(input[i] & 0xff);
		output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}

/**
 *功能描述： 把输入的unsigned char数据解码后输出成UINT4，假设长度len是4的倍数
 *入口参数： 
 *			UINT4 *output;
 *			unsigned char *input;
 *			unsigned int len;
 *返回值：  无
 */
void MD5::innerDecode (UINT4 * output, unsigned char * input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
				 (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
	}
}

/**
 *功能描述：两个内存块数据之间复制 
 *入口参数： 
 *			POINTER output   要复制到的目的数据 
 *			POINTER input    要复制的源数据   
 *			unsigned int len 要复制的长度
 *返回值：   无
 */
void MD5::MD5_memcpy (POINTER output, POINTER input, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		output[i] = input[i];
}

/**
 *功能描述： 把一块内存全部初始化为某一个值
 *入口参数： 
 *			POINTER output   要初始化的内存块
 *			int value        初值
 *			unsigned int len 要初始化的长度
 *返回值：   无
 */
void MD5::MD5_memset (POINTER output, int value, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		((char *)output)[i] = (char)value;
}

/**
 * 功能描述： 供外部使用的加密函数
 * 入口参数：
 *           unsigned char* p_pszDest
 *           const char* p_pszSrc
 * 返回值：  无
 */
void MD5::encode(unsigned char* p_pszDest, const char* p_pszSrc)
{
	MD5_CTX context;
	unsigned int len = strlen (p_pszSrc);

	MD5Init (&context);
	MD5Update (&context, (unsigned char* )p_pszSrc, len);
	MD5Final (p_pszDest, &context);
}

void MD5::Hash64(const char* sInputUrl, char* sOutputDocID)
{
    char sLine[16];
    char szResult[8];
    memset(sLine, 0, 16);
    
    encode((unsigned char* )sLine, (const char* )sInputUrl);  
	memcpy(szResult, sLine, 8);

#ifdef WIN32
    __int64 ddwDocID = *((__int64 *)(szResult));
    _i64toa(ddwDocID, sOutputDocID, 10); //转换为10进制数
#else
    long long ddwDocID = *((long long *)(szResult));
    sprintf (sOutputDocID, "%llu", ddwDocID);
#endif
}

void MD5::Hash64(const char* sInputUrl, long long& ddwDocID)
{
    char sLine[16];
    char szResult[8];
    memset(sLine, 0, 16);

    encode((unsigned char* )sLine, (const char* )sInputUrl);  
    memcpy(szResult, sLine, 8);

    ddwDocID = *((long long *)(szResult));
}

//#define __MD5_MAIN__
#ifdef __MD5_MAIN__
void main()
{
	unsigned char digest[16];
	char *string = "hello world";

	MD5 md5;
	md5.encode(digest, string);

	std::cout << "Before MD5: " << string << std::endl;
	std::cout << "After MD5: " << digest << std::endl;
 
}
#endif
/////////////////////////////////////////////////////////////////////////////////
//本文件结束
/////////////////////////////////////////////////////////////////////////////////
