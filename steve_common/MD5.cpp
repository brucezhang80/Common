/////////////////////////////////////////////////////////////////////////////////
//�ļ�����  MD5.cpp
//����������ʵ����md5���ܺ���
//�㷨˵����
//			����ժҪ(MD��MessageDigest�������ǽ��ɱ䳤�ȵı���M��Ϊ����ɢ�к������룬Ȼ���
//          ��һ���̶����ȵı�־H(M)��H(M)ͨ����Ϊ����ժҪ(MD)������Ҫ������������� 
//			ͨ��˫������һ���������Կ�����Ͷ��Ƚ�����M�����ɢ�к���H�������H��M����MD��
//			���ó������Կ��MD���м��ܣ������ܵ�MD׷���ڱ���M�ĺ��棬���͵����ܶˡ����ն���
//			��ȥ׷���ڱ���M������ܵ�MD������֪��ɢ�к�������H(M)�������Լ�ӵ�е���ԿK�Լ���
//			��MD���ܶ��ó���ʵ��MD���Ƚϼ������H(M)��MD����һ�£����յ��ı���M����ġ�
//ע��ʱ�䣺7/27/2001
/////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "MD5.h"

#define S11 7    //����md5�任ʱҪ�õ��ĳ�������
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

static unsigned char PADDING[64] = {   //���ǲ�λʱ��һ��Ҫ���ϵ���
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// F, G, H and I �ǻ�����MD5����λ����������X��Y��ZΪ32λ����
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ��x����nλ
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

//��1�ֱ任����[a b c d x s ac]��ʾ���²���a = b + ((a + F(b,c,d) + X[k] + T[ac]) <<< s). 
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b);  }

//��2�ֱ任����[a b c d x s ac]��ʾ���²���a=b + ((a + G(b,c,d) + X[k] + T[ac]) <<< s). 
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

//��3�ֱ任����[a b c d x s ac]��ʾ���²���a = b + ((a + H(b,c,d) + X[k] + T[ac]) <<< s). 
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

//��4�ֱ任����[a b c d x s ac]��ʾ���²���a = b + ((a + I(b,c,d) + X[k] + T[ac]) <<< s). 
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/**
 *���������� MD5 ��ʼ��. ��ʼһ��MD5����, дһ���µ�������
 *��ڲ����� MD5_CTX *context ����������
 *����ֵ��   ��
 */
 void MD5::MD5Init (MD5_CTX *context)
{ 
	//������32λ����������������������ݵĳ��ȣ������ȳ�ʼ��Ϊ0
	context->count[0] = context->count[1] = 0;

	//�ĸ�32λ���� (A,B,C,D) ����������ϢժҪ����ʼ��ʹ�õ���ʮ�����Ʊ�ʾ������
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
}

/**
 *����������MD5��ĸ��²���������MD5����ժҪ��������������ı��Ŀ飬������������ 
 *��ڲ����� 
 *			 MD5_CTX *context       ����������
 *			 unsigned char * input  �����ܵ�����
 *			 unsigned int inputLen  �����ܵ����ݵĳ���
 *����ֵ��   ��
 */
void MD5::MD5Update (MD5_CTX * context, unsigned char * input, unsigned int inputLen)
{
	unsigned int i, index, partLen;

	//��ģ64�����ֽ���
	index = (unsigned int)((context->count[0] >> 3) & 0x3F);

	// Update number of bits 
	if ((context->count[0] += ((UINT4)inputLen << 3)) < ((UINT4)inputLen << 3))
		context->count[1]++;
	context->count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	if (inputLen >= partLen) 
	{
		// ��64�������µĲ����Ƚ��м���
		MD5_memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
		MD5Transform (context->state, context->buffer);

		// ��64���ֽ�Ϊһ����λ�����б任
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
 *���������� MD5����������д����ժҪ�����������
 *��ڲ����� 
 *			unsigned char digest[16], 
 *			MD5_CTX * context
 *����ֵ��  ��
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

	// ����״̬��ժҪ
	innerEncode (digest, context->state, 16);

	// ��x��ָ����ڴ����0
	MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/**
 *����������MD5����Ҫ�任���� 
 *��ڲ����� 
 *			UINT4 state[4]           �ĸ�32λ����������������ϢժҪ
 *			unsigned char block[64]  
 *����ֵ��  ��
 *�㷨˵����512λ�ı������ݷֳ�4��128λ�����ݿ飬���α��͵���ͬ��ɢ�к�������4�ּ��㣻
 *          ÿһ�ֶ���32λ��С���ݿ���и��ӵ����㣬������ɺ�ȫ��д��ABCD�Ĵ�����������
 *          һ�ֵļ��㡣�ȵ���������ϣ��ڼĴ���ABCD�е����ݾ�������Ҫ���MD5����
 */
void MD5::MD5Transform (UINT4 state[4], unsigned char block[64])
{
	UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	//ÿһ�Σ�������ԭ�Ĵ����16��Ԫ�ص�����X��
	innerDecode (x, block, 64);

	// ��һ�ֱ任
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

	// �ڶ��ֱ任
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

	// �����ֱ任
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

	// �����ֱ任
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

	//��x��ָ����ڴ����0
	MD5_memset ((POINTER)x, 0, sizeof (x));
}

/**
 *���������� �������UINT4���ݱ���������unsigned char�����賤��len��4�ı���
 *��ڲ����� 
 *			unsigned char *output;
 *			UINT4 *input;
 *			unsigned int len 
 *����ֵ��   ��
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
 *���������� �������unsigned char���ݽ���������UINT4�����賤��len��4�ı���
 *��ڲ����� 
 *			UINT4 *output;
 *			unsigned char *input;
 *			unsigned int len;
 *����ֵ��  ��
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
 *���������������ڴ������֮�临�� 
 *��ڲ����� 
 *			POINTER output   Ҫ���Ƶ���Ŀ������ 
 *			POINTER input    Ҫ���Ƶ�Դ����   
 *			unsigned int len Ҫ���Ƶĳ���
 *����ֵ��   ��
 */
void MD5::MD5_memcpy (POINTER output, POINTER input, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		output[i] = input[i];
}

/**
 *���������� ��һ���ڴ�ȫ����ʼ��Ϊĳһ��ֵ
 *��ڲ����� 
 *			POINTER output   Ҫ��ʼ�����ڴ��
 *			int value        ��ֵ
 *			unsigned int len Ҫ��ʼ���ĳ���
 *����ֵ��   ��
 */
void MD5::MD5_memset (POINTER output, int value, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		((char *)output)[i] = (char)value;
}

/**
 * ���������� ���ⲿʹ�õļ��ܺ���
 * ��ڲ�����
 *           unsigned char* p_pszDest
 *           const char* p_pszSrc
 * ����ֵ��  ��
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
    _i64toa(ddwDocID, sOutputDocID, 10); //ת��Ϊ10������
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
//���ļ�����
/////////////////////////////////////////////////////////////////////////////////
