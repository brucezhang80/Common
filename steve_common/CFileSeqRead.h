/************************************************************
Copyright (C), 1998-2007, Tencent Technology Commpany Limited

�ļ�����: CFileSeqRead.h
����: glenliu
����: 2007.1.29
�汾: 1.0
ģ������: �ļ�˳���������
�����: 
CFileSeqRead
�޸���ʷ:
<author>    <time>   <version >   <desc>
glenliu   2007.1.29    1.0        ����
*************************************************************/
#ifndef __CFILESEQREAD_H__
#define __CFILESEQREAD_H__

#include "twsetypedef.h"


#define MAX_FILE_NAME_LENGTH_READ 500

/********************************************************************
��������: �ļ�˳���������
��Ҫ����ӿ�: 
Creat()
Open()
Read()
Close()
Destory()
*********************************************************************/
class CFileSeqRead
{
public:
	/********************************************************************
	��������: ��������
	�������: dwBuffSize ���ļ�buffer��С
	���� :  ��NULL, �´�������ָ��; NULL, ��������ʧ��. 
	*********************************************************************/
	static CFileSeqRead* Create(const TUINT32 udwBuffSize);

	/********************************************************************
	��������: ��ָ���ļ�
	�������: szFileName  �ļ���
	���� :  ���ļ��Ƿ�ɹ�, TRUE, �ɹ�; FALSE,ʧ��. 
	*********************************************************************/
	TBOOL Open(const TCHAR* szFileName);

	/********************************************************************
	��������: �ر��ļ�
	�������: ��
	���� :  ��
	*********************************************************************/
	TVOID Close();

	/********************************************************************
	��������: ��������
	�������: pFileReader ����������CFileSeqRead����ָ��, ����ִ�����, pFileReader=NULL
	���� :  ��. 
	*********************************************************************/
	static TVOID Destroy(CFileSeqRead*& pFileReader);

	/********************************************************************
	��������: ���ļ�
	�������: szDestBuffer ����������ݵ�Ŀ��bufferָ��
	�������: udwReadSize  �������ݴ�С
	���� :  ʵ�ʶ������ݴ�С.
	*********************************************************************/
	TINT32 Read(TCHAR* szDestBuffer, const TUINT32 udwReadSize);

	/********************************************************************
	��������: ���ļ�(�����ڴ�����ݶ�)
	�������: szDestBuffer ����������ݵ�Ŀ��bufferָ��
	�������: udwReadSize  �������ݴ�С
	���� :  ʵ�ʶ������ݴ�С.
	*********************************************************************/
	TINT32 ReadHuge(TCHAR* szDestBuffer, const TUINT32 udwReadSize);

	/********************************************************************
	��������: ���ļ��ж���һЩ������������
	�������: ��Ӧ��������ָ��
	���� :  ʵ�ʶ��������ֽ���.
	*********************************************************************/
	//CHAR
	inline TINT32 ReadCHAR(TCHAR* pcDest)
	{
		return Read(pcDest, m_udwCHARSize);
	}

	//INT8
	inline TINT32 ReadINT8(TINT8* pbyDest)
	{
		return Read((TCHAR*)pbyDest, m_udwINT8Size);
	}

	//INT16
	inline TINT32 ReadINT16(TINT16* pwDest)
	{
		return Read((TCHAR*)pwDest, m_udwINT16Size);
	}

	//INT32
	inline TINT32 ReadINT32(TINT32* pdwDest)
	{
		return Read((TCHAR*)pdwDest, m_udwINT32Size);
	}

	//INT64
	inline TINT32 ReadINT64(TINT64* pddwDest)
	{
		return Read((TCHAR*)pddwDest, m_udwINT64Size);
	}

	//UCHAR
	inline TINT32 ReadUCHAR(TUCHAR* pucDest)
	{
		return Read((TCHAR*)pucDest, m_udwUCHARSize);
	}

	//UINT8
	inline TINT32 ReadUINT8(TUINT8* pubyDest)
	{
		return Read((TCHAR*)pubyDest, m_udwUINT8Size);
	}

	//UINT16
	inline TINT32 ReadUINT16(TUINT16* puwDest)
	{
		return Read((TCHAR*)puwDest, m_udwUINT16Size);
	}

	//UINT32
	inline TINT32 ReadUINT32(TUINT32* pudwDest)
	{
		return Read((TCHAR*)pudwDest, m_udwUINT32Size);
	}

	//UINT64
	inline TINT32 ReadUINT64(TUINT64* puddwDest)
	{
		return Read((TCHAR*)puddwDest, m_udwUINT64Size);
	}

	//Float32(float)
	inline TINT32 ReadFloat32(TFLOAT32* pfDest)
	{
		return Read((TCHAR*)pfDest, m_udwFloat32Size);
	}

	//Float64(double)
	inline TINT32 ReadFloat64(TFLOAT64* pfDest)
	{
		return Read((TCHAR*)pfDest, m_udwFloat64Size);
	}

	//String(char*)
	inline TINT32 ReadString(TCHAR* szDest, TUINT32 udwStrLen)
	{
		TINT32 dwReadLen = Read(szDest, udwStrLen);
		if ( -1 != dwReadLen )
		{
			szDest[dwReadLen] = '\0';
			return dwReadLen;
		}
		else
		{
			szDest[0] = '\0';
			return -1;
		}
		return dwReadLen;
	}

	/********************************************************************
	��������: ���캯��
	�������: ��
	���� :  ��
	*********************************************************************/
	CFileSeqRead();

	/********************************************************************
	��������: ��������
	�������: ��
	���� :  ��
	*********************************************************************/
	~CFileSeqRead();

	TCHAR * GetFileName() const { return (TCHAR *)m_szFileName; };

	TBOOL IsOpen() { return (m_hFile >= 0); };

private:
	//�ļ���
	TCHAR  m_szFileName[MAX_FILE_NAME_LENGTH_READ];

	//�ļ����
	TINT32 m_hFile;

	//Bufferָ��
	TCHAR* m_szBuffer;

	//Buffer��С
	TUINT32 m_udwSizeOfBuff;

	//Buffer��Ч���ݽ���λ��,m_dwUsedSizeOfBuff<=m_dwSizeOfBuff
	TUINT32 m_udwUsedSizeOfBuff;

	//Buffer�Ѷ����ݽ���λ��,m_dwReadedSizeOfBuff<=m_dwUsedSizeOfBuff
	TUINT32 m_udwReadedSizeOfBuff;

	//һЩ������������Size
	//CHAR
	static TUINT32 m_udwCHARSize;

	//INT8
	static TUINT32 m_udwINT8Size;

	//INT16
	static TUINT32 m_udwINT16Size;

	//INT32
	static TUINT32 m_udwINT32Size;

	//INT64
	static TUINT32 m_udwINT64Size;

	//UCHAR
	static TUINT32 m_udwUCHARSize;

	//UINT8
	static TUINT32 m_udwUINT8Size;

	//UINT16
	static TUINT32 m_udwUINT16Size;

	//UINT32
	static TUINT32 m_udwUINT32Size;

	//UINT64
	static TUINT32 m_udwUINT64Size;

	//Float32(float)
	static TUINT32 m_udwFloat32Size;

	//FLOAT64(double)
	static TUINT32 m_udwFloat64Size;
};


#endif  //__CFILESEQREAD_H__


