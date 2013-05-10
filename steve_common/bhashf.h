/*
 * @File: bhashf.h
 * @Desc: head file of Binary-Hash File
 * @Author: stevetang
 * @History:
 *      2009-04-23   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _BHASHF_H
#define _BHASHF_H

#include "bfile.h"
#include "CFileSeqWrite.h"

typedef int (* pBHashFGetBucketID)(const void *);
typedef int (* pBHashFCmpKey)(const void *, const void *);
typedef int (* pBHashFEnumIdx)(unsigned int, const void *);
typedef int (* pBHashFEnumData)(unsigned int, void *, unsigned int, unsigned char *);
typedef int (* pBHashFEnumDataEx)(unsigned int, void *, unsigned int, unsigned char *);

#pragma pack(1)

typedef struct _tagBHashFIdxInfo {
	unsigned char f_id; /* �����ļ���� */
	unsigned char num; /* Ͱ��������Ŀ */
} recBHashFIdxInfo;

typedef struct _tagBHashFInfo {
	char name_[128]; /* �ļ��� */
	unsigned int isize_; /* Ͱ��Ŀ */
	unsigned int ilen_; /* ��ʼ�����ļ���Ԫ���� */
	unsigned char klen_; /* ��ֵ���� */
	unsigned int dlen_; /* ���ݳ��� */
} recBHashFInfo;

#pragma pack()

#define BHASHF_SUFFIX_INFO      ".info"
#define BHASHF_SUFFIX_IDX		".idx"
#define BHASHF_SUFFIX_DATA		".dat"

class CBaseBHashF
{
	char m_path[128]; /* ·���� */
	char m_fname[128]; /* �ļ���ǰ׺ */

	enuBFileType m_openType; /* ������ */

public:
	CBaseBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024, int max_fcnt = 10, int mul = 2, int max_size = 0);
	virtual ~CBaseBHashF();

protected:
	pBHashFGetBucketID m_get_bucket_id; /* ��ȡͰ��� */
	pBHashFCmpKey m_key_cmp; /* ��ֵ�Ƚ� */

	recBHashFInfo m_bHashF; /* �ļ���Ϣ */

	CMBFile m_idxInfo; /* ������Ϣ */
	CBFile m_dataInfo; /* ���������ļ� */

	int m_max_fcnt; /* ���������չ�ļ��� */
	int m_mul; /* �������� */
	CBFileEx ** m_dataInfoEx; /* ��չ�����ļ���Ϣ */

	int m_buflen; /* �������ݴ�С */
	char * m_buf; /* ���� */
	char * m_tmp_buf; /* ��ʱ���ݴ洢 */

private:
    /*
     * Function: IsExist
     * Desc: ��������Ƿ����
     * In: 
     *     const char *    path    �ļ���
     *     const char *    fname   �ļ���
     * Out: 
     *     none
     * Return code: 
     *      0                   ������
     *      1                   ����
     */
	bool IsExist(const char * path, const char * fname);

	/*
	 * Function: WriteData
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	virtual int WriteData(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: WriteDataEx
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	virtual int WriteDataEx(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: ReadData
	 * Desc: ������
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	virtual int ReadData(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: ReadDataEx
	 * Desc: ������
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	virtual int ReadDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: DelData
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	virtual int DelData(int site, recBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

	/*
	 * Function: DelDataEx
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	virtual int DelDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

protected:
	/*
	 * Function: GetData
	 * Desc: ��ȡ����
	 * In: 
	 *     const void *   key         ��ֵ
	 * Out: 
	 *     void *         data        ����
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int GetData(const void * buf, int num, int ilen, const void * key, char * &data);

	/*
	 * Function: WriteDataEx
	 * Desc: д����
	 * In: 
	 *     int             site     Ͱ
     *     const void *    data     ����
	 *     int             len      ���ݳ���
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *     >0                   �ļ����
	 */
	int WriteDataEx(int site, const void * data, int len);

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
	int Open(const char * path, const char * fname, int bucket = 7, int ilen = 1024, int klen = 0, int dlen = 0, enuBFileType openType = E_READ);

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
     *     const void *    key      ��ֵ
     *     const void *    data     ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Write(const void * key, const void * data, int datalen, unsigned char force = 1);

    /*
     * Function: Delete
     * Desc: ɾ��
     * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Delete(const void * key, void * data = NULL, int * datalen = NULL);

    /*
     * Function: Read
     * Desc: ������
     * In: 
     *     const void *    key      ��ֵ
	 *     int             key_len  ������
     * Out: 
     *     void *          data     ����
	 *     int&            data_len ���ݳ���
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Read(const void * key, void * data, int& datalen);

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

	/*
     * Function: EnumIdx
     * Desc: ö���������ݲ����д���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int EnumIdx(const char * path, const char * fname, pBHashFEnumIdx func);

    /*
     * Function: EnumData
     * Desc: ö�����ݲ����д���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int EnumData(const char * path, const char * fname, pBHashFEnumData func);

    /*
     * Function: EnumDataEx
     * Desc: ö����չ�����ļ������д���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int EnumDataEx(const char * path, const char * fname, pBHashFEnumDataEx func);
};

class CBHashF : public CBaseBHashF
{
public:
	CBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024, int max_fcnt = 10, int mul = 2, int max_size = 0);
	~CBHashF();

public:
    /*
     * Function: Write
     * Desc: д����
     * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Write(const void * key, const void * data, unsigned char force = 1);

    /*
     * Function: Delete
     * Desc: ɾ��
     * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Delete(const void * key, void * data = NULL);

    /*
     * Function: Read
     * Desc: ������
     * In: 
     *     const void *    key      ��ֵ
	 *     int             key_len  ������
     * Out: 
     *     void *          data     ����
	 *     int&            data_len ���ݳ���
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Read(const void * key, void * data);
};

/* �䳤���ݣ��������ݽṹΪ��key+len+data;֧����󳤶�Ϊ65535 */
class CBHashFEx : public CBaseBHashF
{
public:
	CBHashFEx(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024, int max_fcnt = 10, int mul = 2, int max_size = 0);
	~CBHashFEx();

private:
	/*
	 * Function: PreSort
	 * Desc: ������������
	 * In: 
     *     const char *    src      Դ���ݴ�
	 * Out: 
     *     char *          dst      ����
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int PreSort(int num, const char * src, char * dst);

	/*
	 * Function: WriteData
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int WriteData(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: WriteDataEx
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int WriteDataEx(int site, recBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: ReadData
	 * Desc: ������
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int ReadData(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: ReadDataEx
	 * Desc: ������
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int ReadDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: DelData
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int DelData(int site, recBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

	/*
	 * Function: DelDataEx
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int DelDataEx(int site, recBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

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
	int Open(const char * path, const char * fname, int bucket = 7, int ilen = 1024, int klen = 0, enuBFileType openType = E_READ);
};

#pragma pack(1)

typedef struct _tagMinBHashFIdxInfo {
	unsigned char num; /* Ͱ��������Ŀ */
} recMinBHashFIdxInfo;

#pragma pack()

#define MINBHASHF_SUFFIX_INFO      ".info"
#define MINBHASHF_SUFFIX_IDX		".idx"
#define MINBHASHF_SUFFIX_DATA		".dat"

class CBaseMinBHashF
{
	char m_path[128]; /* ·���� */
	char m_fname[128]; /* �ļ���ǰ׺ */

	enuBFileType m_openType; /* ������ */

public:
	CBaseMinBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024);
	virtual ~CBaseMinBHashF();

protected:
	pBHashFGetBucketID m_get_bucket_id; /* ��ȡͰ��� */
	pBHashFCmpKey m_key_cmp; /* ��ֵ�Ƚ� */

	recBHashFInfo m_bHashF; /* �ļ���Ϣ */

	CMBFile m_idxInfo; /* ������Ϣ */
	CBFileEx m_dataInfo; /* ��չ�����ļ���Ϣ */

	int m_buflen; /* �������ݴ�С */
	char * m_buf; /* ���� */
	char * m_tmp_buf; /* ��ʱ���ݴ洢 */

private:
    /*
     * Function: IsExist
     * Desc: ��������Ƿ����
     * In: 
     *     const char *    path    �ļ���
     *     const char *    fname   �ļ���
     * Out: 
     *     none
     * Return code: 
     *      0                   ������
     *      1                   ����
     */
	bool IsExist(const char * path, const char * fname);

	/*
	 * Function: WriteDataEx
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	virtual int WriteData(int site, recMinBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: ReadDataEx
	 * Desc: ������
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	virtual int ReadData(int site, recMinBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: DelData
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	virtual int DelData(int site, recMinBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

protected:
	/*
	 * Function: GetData
	 * Desc: ��ȡ����
	 * In: 
	 *     const void *   key         ��ֵ
	 * Out: 
	 *     void *         data        ����
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int GetData(const void * buf, int num, int ilen, const void * key, char * &data);

	/*
	 * Function: WriteDataEx
	 * Desc: д����
	 * In: 
	 *     int             site     Ͱ
     *     const void *    data     ����
	 *     int             len      ���ݳ���
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *     >0                   �ļ����
	 */
	int WriteData(int site, const void * data, int len);

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
	int Open(const char * path, const char * fname, int bucket = 7, int ilen = 1024, int klen = 0, int dlen = 0, enuBFileType openType = E_READ);

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
     *     const void *    key      ��ֵ
     *     const void *    data     ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Write(const void * key, const void * data, int datalen, unsigned char force = 1);

    /*
     * Function: Delete
     * Desc: ɾ��
     * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Delete(const void * key, void * data = NULL, int * datalen = NULL);

    /*
     * Function: Read
     * Desc: ������
     * In: 
     *     const void *    key      ��ֵ
	 *     int             key_len  ������
     * Out: 
     *     void *          data     ����
	 *     int&            data_len ���ݳ���
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Read(const void * key, void * data, int& datalen);

	/*
     * Function: EnumIdx
     * Desc: ö���������ݲ����д���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int EnumIdx(const char * path, const char * fname, pBHashFEnumIdx func);

    /*
     * Function: EnumDataEx
     * Desc: ö����չ�����ļ������д���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int EnumData(const char * path, const char * fname, pBHashFEnumDataEx func);
};

class CMinBHashF : public CBaseMinBHashF
{
public:
	CMinBHashF(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024);
	~CMinBHashF();

public:
    /*
     * Function: Write
     * Desc: д����
     * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Write(const void * key, const void * data, unsigned char force = 1);

    /*
     * Function: Delete
     * Desc: ɾ��
     * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Delete(const void * key, void * data = NULL);

    /*
     * Function: Read
     * Desc: ������
     * In: 
     *     const void *    key      ��ֵ
	 *     int             key_len  ������
     * Out: 
     *     void *          data     ����
	 *     int&            data_len ���ݳ���
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
	int Read(const void * key, void * data);
};

/* �䳤���ݣ��������ݽṹΪ��key+len+data;֧����󳤶�Ϊ65535 */
class CMinBHashFEx : public CBaseMinBHashF
{
public:
	CMinBHashFEx(pBHashFGetBucketID get_bucket_id, pBHashFCmpKey cmp_key, int buflen = 1024 * 1024);
	~CMinBHashFEx();

private:
	/*
	 * Function: PreSort
	 * Desc: ������������
	 * In: 
     *     const char *    src      Դ���ݴ�
	 * Out: 
     *     char *          dst      ����
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int PreSort(int num, const char * src, char * dst);

	/*
	 * Function: WriteData
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int WriteData(int site, recMinBHashFIdxInfo * info, const void * key, const void * data, int datalen, unsigned char force = 1);

	/*
	 * Function: ReadData
	 * Desc: ������
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int ReadData(int site, recMinBHashFIdxInfo * info, const void * key, void * data, int& datalen);

	/*
	 * Function: DelData
	 * Desc: д����
	 * In: 
     *     const void *    key      ��ֵ
     *     const void *    data     ����
	 * Out: 
	 *     none
	 * Return code: 
	 *     -1                   ����
	 *      0                   �ɹ�
	 */
	int DelData(int site, recMinBHashFIdxInfo * info, const void * key, void * data = NULL, int * datalen = NULL);

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
	int Open(const char * path, const char * fname, int bucket = 7, int ilen = 1024, int klen = 0, enuBFileType openType = E_READ);
};

#endif
