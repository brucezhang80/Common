/************************************************************
Copyright (C), 1998-2007, Tencent Technology Commpany Limited

�ļ�����: CFileSeqRead.cpp
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

#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif //WIN32

#include "CFileSeqRead.h"

//һЩ������������Size��ʼ��ֵ
//CHAR
TUINT32 CFileSeqRead::m_udwCHARSize = sizeof(TCHAR);
//INT8
TUINT32 CFileSeqRead::m_udwINT8Size = sizeof(TINT8);
//INT16
TUINT32 CFileSeqRead::m_udwINT16Size = sizeof(TINT16);
//INT32
TUINT32 CFileSeqRead::m_udwINT32Size = sizeof(TINT32);
//INT64
TUINT32 CFileSeqRead::m_udwINT64Size = sizeof(TINT64);
//UCHAR
TUINT32 CFileSeqRead::m_udwUCHARSize = sizeof(TUCHAR);
//UINT8
TUINT32 CFileSeqRead::m_udwUINT8Size = sizeof(TUINT8);
//UINT16
TUINT32 CFileSeqRead::m_udwUINT16Size = sizeof(TUINT16);
//UINT32
TUINT32 CFileSeqRead::m_udwUINT32Size = sizeof(TUINT32);
//UINT64
TUINT32 CFileSeqRead::m_udwUINT64Size = sizeof(TUINT64);
//Float32(float)
TUINT32 CFileSeqRead::m_udwFloat32Size = sizeof(TFLOAT32);
//FLOAT64(double)
TUINT32 CFileSeqRead::m_udwFloat64Size = sizeof(TFLOAT64);


/********************************************************************
��������: ���캯��
�������: ��
���� :  ��
*********************************************************************/
CFileSeqRead::CFileSeqRead()
{
	m_szFileName[0] = '\0';
	m_hFile = -1;
	m_udwSizeOfBuff = 0;
	m_szBuffer = NULL;
	m_udwUsedSizeOfBuff = 0;
	m_udwReadedSizeOfBuff = 0;
}

/********************************************************************
��������: ��������
�������: ��
���� :  ��
*********************************************************************/
CFileSeqRead::~CFileSeqRead()
{
	if ( NULL != m_szBuffer )
	{
		delete []m_szBuffer;
	}
	m_szBuffer = NULL;

	if ( 0 <= m_hFile )
	{
		close( m_hFile );
	}
	m_hFile = -1;

	m_udwSizeOfBuff = 0;
	m_udwUsedSizeOfBuff = 0;
	m_udwReadedSizeOfBuff = 0;
}

/********************************************************************
��������: ��������
�������: dwBuffSize ���ļ�buffer��С
���� :  ��NULL, �´�������ָ��; NULL, ��������ʧ��. 
*********************************************************************/
CFileSeqRead* CFileSeqRead::Create(const TUINT32 udwBuffSize)
{
	if ( 0 >= udwBuffSize )
		return NULL;

	//����Buffer
	TCHAR* szBuffer = new TCHAR[udwBuffSize];
	if ( NULL == szBuffer )
		return NULL;

	//����Reader����
	CFileSeqRead* pFileReader = new CFileSeqRead;
	if ( NULL == pFileReader )
		return NULL;
	
	pFileReader->m_hFile = -1;
	pFileReader->m_szBuffer = szBuffer;
	pFileReader->m_udwSizeOfBuff = udwBuffSize;
	pFileReader->m_udwUsedSizeOfBuff = 0;
	pFileReader->m_udwReadedSizeOfBuff = 0;

	return pFileReader;
}

/********************************************************************
��������: ��ָ���ļ�
�������: szFileName  �ļ���
���� :  ���ļ��Ƿ�ɹ�, TRUE, �ɹ�; FALSE,ʧ��. 
*********************************************************************/
TBOOL CFileSeqRead::Open(const TCHAR* szFileName)
{
	if ( 0 <= m_hFile )
	{
		//�Ѵ��ļ���δ�ر�
		return FALSE;
	}

	//��ֻ����ʽ���ļ�
#ifdef WIN32
	m_hFile = open( szFileName, O_RDONLY | O_BINARY | O_SEQUENTIAL, S_IREAD );
#else
	m_hFile = open( szFileName, O_RDONLY, S_IREAD );
#endif //WIN32
	if ( m_hFile == -1 )
	{
		//���ļ�ʧ��
		return FALSE;
	}
	//��¼�ļ���
	strcpy( m_szFileName, szFileName );

	//��ʼ��Buffer
	memset( m_szBuffer, 0, sizeof(TCHAR)*m_udwSizeOfBuff );

	//������ʼ��
	m_udwUsedSizeOfBuff = 0;
	m_udwReadedSizeOfBuff = 0;

	//����
	return TRUE;
}

/********************************************************************
��������: �ر��ļ�
�������: ��
���� :  ��
*********************************************************************/
TVOID CFileSeqRead::Close()
{
	//��ʼ��Buffer
	memset( m_szBuffer, 0, sizeof(TCHAR)*m_udwSizeOfBuff );

	//������ʼ��
	m_udwUsedSizeOfBuff = 0;
	m_udwReadedSizeOfBuff = 0;

	//�ر��ļ�
	if ( 0 <= m_hFile )
	{
		close(m_hFile);
	}

	m_hFile = -1;

	return;
}

/********************************************************************
��������: ��������
�������: pFileReader ���ļ�����ָ��
���� :  ��. 
*********************************************************************/
TVOID CFileSeqRead::Destroy(CFileSeqRead*& pFileReader)
{
	if ( NULL == pFileReader)
	{
		return;
	}

	if ( 0 <= pFileReader->m_hFile )
	{
		//�����ļ���δ�ر�
		pFileReader->Close();
	}

	delete pFileReader;

	pFileReader = NULL;

	return;
}

/********************************************************************
��������: ���ļ�
�������: szDestBuffer ����������ݵ�Ŀ��bufferָ��
�������: udwReadSize  �������ݴ�С
���� :  ʵ�ʶ������ݴ�С.
*********************************************************************/
TINT32 CFileSeqRead::Read(TCHAR* szDestBuffer, const TUINT32 udwReadSize)
{
	if ( m_udwUsedSizeOfBuff - m_udwReadedSizeOfBuff >= udwReadSize )
	{
		//���Ѷ�Buffer�ж�ȡ
		memcpy( szDestBuffer, (m_szBuffer+m_udwReadedSizeOfBuff), udwReadSize);
		m_udwReadedSizeOfBuff += udwReadSize;
		return (TINT32)udwReadSize;
	}
	else
	{
		//�ȴ��Ѷ�Buffer�ж�ȡһ����
		TUINT32 udwReadedDest = m_udwUsedSizeOfBuff-m_udwReadedSizeOfBuff;		//�Ѷ�Buffer��С
		memcpy(szDestBuffer, (m_szBuffer+m_udwReadedSizeOfBuff), udwReadedDest);
		//���ļ�������һ��
		m_udwReadedSizeOfBuff = 0;
		TINT32 dwNewReadFileSize = read(m_hFile, m_szBuffer, m_udwSizeOfBuff);
		if ( 0 >= dwNewReadFileSize )
		{
			//�����ļ���β��m_hFile��Ч���ļ�δ�򿪻�����
			m_udwUsedSizeOfBuff = 0;
			return -1;
		}
		else
		{
			//�ɹ�����������
			m_udwUsedSizeOfBuff = (TUINT32)dwNewReadFileSize;
		}
		
		//�ݹ����Read
		TINT32 dwIteraReadSize = Read(szDestBuffer+udwReadedDest,udwReadSize-udwReadedDest);
		if ( -1 == dwIteraReadSize )
		{
			//�ݹ���ļ�����
			return -1;
		}
		else
		{
			//�ݹ���ļ��ɹ�
			return  ( dwIteraReadSize + (TINT32)udwReadedDest );
		}
	}
	return -1;
}

/********************************************************************
��������: ���ļ�(���)
�������: szDestBuffer ����������ݵ�Ŀ��bufferָ��
�������: udwReadSize  �������ݴ�С
���� :  ʵ�ʶ������ݴ�С.
*********************************************************************/
TINT32 CFileSeqRead::ReadHuge(TCHAR* szDestBuffer, const TUINT32 udwReadSize)
{
	if ( m_udwUsedSizeOfBuff - m_udwReadedSizeOfBuff >= udwReadSize )
	{
		//���Ѷ�Buffer�ж�ȡ
		memcpy( szDestBuffer, (m_szBuffer+m_udwReadedSizeOfBuff), udwReadSize);
		m_udwReadedSizeOfBuff += udwReadSize;		//Buff����Ч�����Ѷ���С�ۼ�
		return ((TINT32)udwReadSize);
	}
	else
	{
		//�ȴ��Ѷ�Buffer�ж�ȡһ����
		TUINT32 udwReadedDest = m_udwUsedSizeOfBuff-m_udwReadedSizeOfBuff;		//�Ѷ�Buffer��С
		memcpy(szDestBuffer, (m_szBuffer+m_udwReadedSizeOfBuff), udwReadedDest);
		m_udwReadedSizeOfBuff = 0;					//Buff����Ч�����Ѷ���С����
		m_udwUsedSizeOfBuff = 0;					//Buff����Ч���ݴ�С����
		//���ļ���ʣ�ಿ��һ�ζ���Ŀ��Buffer
		TINT32 dwNewReadFileSize = read(m_hFile, szDestBuffer+udwReadedDest, udwReadSize-udwReadedDest);
		if ( dwNewReadFileSize < (TINT32)(udwReadSize-udwReadedDest) )
		{
			//���ļ����С��Ҫ���С������
			return (-1);
		}
		else
		{
			//���ض����ֽ���
			return  ( dwNewReadFileSize + (TINT32)udwReadedDest );
		}
	}
	return -1;
}

/********************************************************************
��������: ���ļ��ж���һЩ������������
�������: ��Ӧ��������ָ��
���� :  ʵ�ʶ��������ֽ���.
*********************************************************************/
/*
//INT32
TINT32 CFileSeqRead::ReadINT32(TINT32* pdwDest)
{
	return Read((TCHAR*)pdwDest, m_udwINT32Size);
}

//INT64
TINT32 CFileSeqRead::ReadINT64(TINT64* pddwDest)
{
	return Read((TCHAR*)pddwDest, m_udwINT64Size);
}

//UINT32
TINT32 CFileSeqRead::ReadUINT32(TUINT32* pudwDest)
{
	return Read((TCHAR*)pudwDest, m_udwUINT32Size);
}

//UINT64
TINT32 CFileSeqRead::ReadUINT64(TUINT64* puddwDest)
{
	return Read((TCHAR*)puddwDest, m_udwUINT64Size);
}

//Float32(float)
TINT32 CFileSeqRead::ReadFloat32(TFLOAT32* pfDest)
{
	return Read((TCHAR*)pfDest, m_udwFloat32Size);
}

//Float64(double)
TINT32 CFileSeqRead::ReadFloat64(TFLOAT64* pfDest)
{
	return Read((TCHAR*)pfDest, m_udwFloat64Size);
}

//String(char*)
TINT32 CFileSeqRead::ReadString(TCHAR* szDest, TUINT32 udwStrLen)
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

//CHAR
TINT32 CFileSeqRead::ReadCHAR(TCHAR* pcDest)
{
	return Read(pcDest, m_udwCHARSize);
}

//INT8
TINT32 CFileSeqRead::ReadINT8(TINT8* pbyDest)
{
	return Read((TCHAR*)pbyDest, m_udwINT8Size);
}

//INT16
TINT32 CFileSeqRead::ReadINT16(TINT16* pwDest)
{
	return Read((TCHAR*)pwDest, m_udwINT16Size);
}

//UCHAR
TINT32 CFileSeqRead::ReadUCHAR(TUCHAR* pucDest)
{
	return Read((TCHAR*)pucDest, m_udwUCHARSize);
}

//UINT8
TINT32 CFileSeqRead::ReadUINT8(TUINT8* pubyDest)
{
	return Read((TCHAR*)pubyDest, m_udwUINT8Size);
}

//UINT16
TINT32 CFileSeqRead::ReadUINT16(TUINT16* puwDest)
{
	return Read((TCHAR*)puwDest, m_udwUINT16Size);
}
*/

