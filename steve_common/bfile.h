/*
 * @File: bfile.h
 * @Desc: head file of Bitmap File
 * @Author: stevetang
 * @History:
 *      2009-04-28   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _BFILE_H
#define _BFILE_H

#pragma pack(1)

typedef enum {
	E_READ = 0,
	E_WRITE
} enuBFileType;

typedef struct _tagMBFile {
	char name_[128]; /* �ļ��� */
	unsigned int isize_; /* �ڵ��� */
	unsigned int ilen_; /* �ڵ㵥Ԫ���� */
	unsigned char data_[0]; /* ������ */	
} recMBFile;

#pragma pack()

class CMBFile
{
	int m_fd; /* �ļ���� */
	enuBFileType m_openType; /* ������ */
	recMBFile * m_bFile; /* ����ļ���Ϣ */

public:
	CMBFile();
	~CMBFile();

public:
    /*
     * Function: Open
     * Desc: ���ļ�
     * In: 
     *     const char *    fname   �ļ���
	 *     int             bucket  Ͱ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Open(const char * path, int isize, int ilen, enuBFileType openType = E_READ);

    /*
     * Function: Close
     * Desc: �ر��ļ�
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Close();

    /*
     * Function: Read
     * Desc: ������
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     NULL                ����
     *     ��NULL              �ɹ�
     */
	void * Read(unsigned int site);
};

#pragma pack(1)

typedef struct _tagBFile {
	char name_[128]; /* �ļ��� */
	unsigned int isize_; /* �ڵ��� */
	unsigned int ilen_; /* �ڵ㵥Ԫ���� */
	unsigned long long addr_; /* ���ݵ�ַ */
	unsigned char binfo_[0]; /* �ڵ��bitmap��Ϣ */
	/* ������ */
} recBFile;

#pragma pack()

#define BFILE_GET_VALID(info_, site, bit) \
	{ \
		bit = ((info_[site / 8] >> (site % 8)) & 0x01); \
	}

#define BFILE_SET_VALID(info_, site, bit) \
	{ \
		if (bit) \
			info_[site / 8] |= (0x01 << (site % 8)); \
		else \
			info_[site / 8] &= ~(0x01 << (site % 8)); \
	}

class CBFile
{
	int m_fd; /* �ļ���� */
	enuBFileType m_openType; /* ������ */
	recBFile * m_bFile; /* ����ļ���Ϣ */

	int m_buflen; /* �������ݴ�С */
	char * m_buf; /* ��ʱ���ݻ��� */

	/* ӳ�䲿�����ݣ����ڴ����������� */
	unsigned int m_start; /* ӳ�����ʼͰ�� */
	unsigned int m_end; /* ӳ��Ľ���Ͱ�� */

	int m_maxsize; /* ���ݴ�С */
	char * m_data; /* ӳ��������� */
	unsigned char m_dirty; /* �Ƿ�Ϊ������ */
	unsigned char m_attach; /* �������� */

public:
	CBFile(int buflen = 1024*1024, int max_size = 0);
	~CBFile();

private:
    /*
     * Function: IsValidData
     * Desc: �Ƿ������Ч����
     * In: 
     *     unsigned int    site    λ��
     * Out: 
     *     none
     * Return code: 
     *     1                   ��
     *     0                   ��
     */
	unsigned char IsValidData(unsigned int site);

    /*
     * Function: IsValidData
     * Desc: �Ƿ������Ч����
     * In: 
     *     unsigned int    site    λ��
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int SetValidData(unsigned int site, unsigned char bit);

public:
    /*
     * Function: Open
     * Desc: ���ļ�
     * In: 
     *     const char *    fname   �ļ���
	 *     int             bucket  Ͱ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Open(const char * path, int isize, int ilen, enuBFileType openType = E_READ);

    /*
     * Function: Close
     * Desc: �ر��ļ�
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Close();

    /*
     * Function: SetChunk
     * Desc: ���������
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int SetChunk(int start, int end);

    /*
     * Function: Write
     * Desc: д����
     * In: 
     *     const void *    data     ����
	 *     int             data_len ���ݳ���
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Write(unsigned int site, const void * data, int data_len, unsigned char force = 1);

    /*
     * Function: Read
     * Desc: ������
     * In: 
     *     none
     * Out: 
     *     void *          data     ����
	 *     int&            data_len ���ݳ���
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Read(unsigned int site, void * data, int& data_len);

    /*
     * Function: Flush
     * Desc: ��ʱ��������д��ʵ��������
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Flush();

    /*
     * Function: Attach
     * Desc: ʹ�������Ļ���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Attach(int maxsize, char * data);

    /*
     * Function: Detach
     * Desc: ��ʱ��������д��ʵ��������
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Detach();
};

#pragma pack(1)

typedef struct _tagBFileEx {
	char name_[128]; /* �ļ��� */
	unsigned int isize_; /* �ڵ��� */
	unsigned int ilen_; /* �ڵ㵥Ԫ���� */
	unsigned int inode_; /* ��ǰ�ڵ��� */
	unsigned long long addr_; /* ���ݵ�ַ */
	struct _tagBInfo {
		unsigned int offset_ : 31; /* ������ƫ�ƣ���0��ʼ���� */
		unsigned int bit_ : 1; /* ���Ƿ���Ч */
	} binfo_[0]; /* �ڵ���Ϣ */
	/* ������ */
} recBFileEx;

#pragma pack()

#define BFILEEX_GET_VALID(info_, site, bit, offset) \
	{ \
		bit = info_[site].bit_; \
		if (bit == 0x01) \
			offset = info_[site].offset_; \
	}

#define BFILEEX_SET_VALID(info_, site, bit, offset) \
	{ \
		info_[site].bit_ = bit; \
		info_[site].offset_ = offset; \
	}

class CBFileEx
{
	int m_fd; /* �ļ���� */
	enuBFileType m_openType; /* ������ */
	recBFileEx * m_bFile; /* ����ļ���Ϣ */

	int m_buflen; /* �������ݴ�С */
	char * m_buf; /* ���ݻ��� */

public:
	CBFileEx(int buflen = 1024*1024);
	~CBFileEx();

private:
    /*
     * Function: GetOffset
     * Desc: ȡ��������ƫ��
     * In: 
     *     unsigned int    site    λ��
     * Out: 
     *     unsigned int&   offset  ƫ��
     * Return code: 
     *     1                   ��
     *     0                   ��
     */
	unsigned char GetOffset(unsigned int site, unsigned int& offset);

    /*
     * Function: SetOffset
     * Desc: ����ƫ��
     * In: 
     *     unsigned int    site    λ��
     *     unsigned char   valid   �Ƿ���Ч
     *     unsigned int    offset  ƫ��
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int SetOffset(unsigned int site, unsigned char valid, unsigned int offset);

public:
    /*
     * Function: Open
     * Desc: ���ļ�
     * In: 
     *     const char *    fname   �ļ���
	 *     int             bucket  Ͱ
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Open(const char * path, int isize, int ilen, enuBFileType openType = E_READ);

    /*
     * Function: Close
     * Desc: �ر��ļ�
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Close();

    /*
     * Function: Write
     * Desc: д����
     * In: 
     *     const void *    data     ����
	 *     int             data_len ���ݳ���
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Write(unsigned int site, const void * data, int data_len, unsigned char force = 1);

    /*
     * Function: Read
     * Desc: ������
     * In: 
     *     none
     * Out: 
     *     void *          data     ����
	 *     int&            data_len ���ݳ���
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Read(unsigned int site, void * data, int& data_len);
};

#endif
