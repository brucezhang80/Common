////////////////////////////////////////////////////////////////////////////////
// ��Ȩ��Ϣ��(C) 2003, Jerry.CS.HUST.China
// �ļ�����  MD5.h
// ���ߣ�    Jerry
// ���������������ݽ���MD5����
// �汾��    V1.1.2003.02.25
// �޸���ʷ��
////////////////////////////////////////////////////////////////////////////////

#ifndef _MD5_H_
#define _MD5_H_

typedef unsigned char *POINTER;
typedef unsigned short int UINT2;
typedef unsigned long int UINT4;

//CDESCrypt��Ķ���
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

	// ���ⲿʹ�õĹ�ϣ����������16bytes��hashֵ
	static void encode(unsigned char* p_pszDest, const char* p_pszSrc);

	// ���ⲿʹ�õĹ�ϣ����������long long���ַ�����ʽhashֵ
    static void Hash64(const char* sInputUrl, char* sOutputDocID);	

	// ���ⲿʹ�õĹ�ϣ����������long long��hashֵ
    static void Hash64(const char* sInputUrl, long long& ddwDocID);

private:
	// MD5 ��ʼ��. ��ʼһ��MD5����, дһ���µ�������
	static void MD5Init (MD5_CTX *);

	// MD5��ĸ��²���������MD5����ժҪ��������������ı��Ŀ飬������������ 
	static void MD5Update(MD5_CTX *, unsigned char *, unsigned int);

	// MD5����������д����ժҪ�����������
	static void MD5Final (unsigned char [16], MD5_CTX *);

	// MD5����Ҫ�任����
	static void MD5Transform (UINT4 [4], unsigned char [64]);

	// �������UINT4���ݱ���������unsigned char
	static void innerEncode (unsigned char *, UINT4 *, unsigned int);

	// �������unsigned char���ݽ���������UINT4
	static void innerDecode (UINT4 *, unsigned char *, unsigned int);

	// �����ڴ������֮�临��,ע�⣺���ﲻ����strcpy
	static void MD5_memcpy (POINTER, POINTER, unsigned int);

	// ��һ���ڴ�ȫ����ʼ��Ϊĳһ��ֵ
	static void MD5_memset (POINTER, int, unsigned int);
	
};

#endif
