/************************************************************
Copyright (C), 1998-2007, Tencent Technology Commpany Limited

�ļ�����: CFileSeqWrite.cpp
����: glenliu
����: 2007.1.29
�汾: 1.0
ģ������: �ļ�˳��д������
�����: 
CFileSeqWrite
�޸���ʷ:
<author>    <time>   <version >   <desc>
glenliu   2007.1.29    1.0        ����
*************************************************************/

#include "CFileSeqWrite.h"
#include "string.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif  //WIN32

//һЩ������������Size��ʼ��ֵ
//CHAR
TUINT32 CFileSeqWrite::m_udwCHARSize = sizeof(TCHAR);
//INT8
TUINT32 CFileSeqWrite::m_udwINT8Size = sizeof(TINT8);
//INT16
TUINT32 CFileSeqWrite::m_udwINT16Size = sizeof(TINT16);
//INT32
TUINT32 CFileSeqWrite::m_udwINT32Size = sizeof(TINT32);
//INT64
TUINT32 CFileSeqWrite::m_udwINT64Size = sizeof(TINT64);
//UCHAR
TUINT32 CFileSeqWrite::m_udwUCHARSize = sizeof(TUCHAR);
//UINT8
TUINT32 CFileSeqWrite::m_udwUINT8Size = sizeof(TUINT8);
//UINT16
TUINT32 CFileSeqWrite::m_udwUINT16Size = sizeof(TUINT16);
//UINT32
TUINT32 CFileSeqWrite::m_udwUINT32Size = sizeof(TUINT32);
//UINT64
TUINT32 CFileSeqWrite::m_udwUINT64Size = sizeof(TUINT64);
//Float32(float)
TUINT32 CFileSeqWrite::m_udwFloat32Size = sizeof(TFLOAT32);
//FLOAT64(double)
TUINT32 CFileSeqWrite::m_udwFloat64Size = sizeof(TFLOAT64);


/********************************************************************
��������: ���캯��
�������: ��
���� :  ��
*********************************************************************/
CFileSeqWrite::CFileSeqWrite()
{
	m_szFileName[0] = '\0';
	m_hFile = -1;
	m_udwSizeOfBuff = 0;
	m_szBuffer = NULL;
	m_udwUsedSizeOfBuff = 0;
}

/********************************************************************
��������: ��������
�������: ��
���� :  ��
*********************************************************************/
CFileSeqWrite::~CFileSeqWrite()
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
}

/********************************************************************
��������: ��������
�������: dwBuffSize ���ļ�buffer��С
���� :  ��NULL, �´�������ָ��; NULL, ��������ʧ��. 
*********************************************************************/
CFileSeqWrite* CFileSeqWrite::Create(const TUINT32 udwBuffSize)
{
	if ( 0 >= udwBuffSize )
		return NULL;

	//����Buffer
	TCHAR* szBuffer = new TCHAR[udwBuffSize];
	if ( NULL == szBuffer )
		return NULL;

	//����Reader����
	CFileSeqWrite* pFileWriter = new CFileSeqWrite;
	if ( NULL == pFileWriter )
		return NULL;

	pFileWriter->m_hFile = -1;
	pFileWriter->m_szBuffer = szBuffer;
	pFileWriter->m_udwSizeOfBuff = udwBuffSize;
	pFileWriter->m_udwUsedSizeOfBuff = 0;

	return pFileWriter;
}

/********************************************************************
��������: ��ָ���ļ�
�������: szFileName  �ļ���
���� :  ���ļ��Ƿ�ɹ�, TRUE, �ɹ�; FALSE,ʧ��. 
*********************************************************************/
TBOOL CFileSeqWrite::Open(const TCHAR* szFileName, const TBOOL bToAppended)
{
	if ( 0 <= m_hFile )
	{
		//�Ѵ��ļ���δ�ر�
		return FALSE;
	}

	//��ֻд��ʽ���ļ�
	if ( bToAppended )
	{
		//׷�ӷ�ʽ
#ifdef WIN32
		m_hFile = open( szFileName, O_APPEND | O_CREAT | O_WRONLY | O_BINARY | O_SEQUENTIAL, S_IWRITE);
#else
		m_hFile = open( szFileName, O_APPEND | O_CREAT | O_WRONLY, 0666 );
#endif //WIN32
	}
	else
	{
		//���Ƿ�ʽ
#ifdef WIN32
		m_hFile = open( szFileName, O_TRUNC | O_CREAT | O_WRONLY | O_BINARY | O_SEQUENTIAL, S_IWRITE);//, S_IWRITE
#else
		m_hFile = open( szFileName, O_TRUNC | O_CREAT | O_WRONLY, 0666 );//, S_IWRITE
#endif
	}
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

	//����
	return TRUE;
}
/********************************************************************
��������: ǿ�ƽ�����������д��Ӳ���ļ�
�������: ��
���� :    TRUE,�ɹ�, FALSE,ʧ��. 
*********************************************************************/
TBOOL CFileSeqWrite::Flush()
{
	if ( 0 > m_hFile )
	{
		return FALSE;
	}

	//Buffer����д��
	if ( 0 > write(m_hFile, m_szBuffer, m_udwUsedSizeOfBuff) )
	{
		//д�̳���
		m_udwUsedSizeOfBuff = 0;
		return FALSE;
	}
	//������ʼ��
	m_udwUsedSizeOfBuff = 0;
	return TRUE;
}

/********************************************************************
��������: �ر��ļ�
�������: ��
���� :  ��
*********************************************************************/
TVOID CFileSeqWrite::Close()
{
	if ( 0 <= m_hFile )
	{
		//Buffer����д��
		write(m_hFile, m_szBuffer, m_udwUsedSizeOfBuff);

		//������ʼ��
		m_udwUsedSizeOfBuff = 0;

		close(m_hFile);
	}
	m_hFile = -1;

	return;
}

/********************************************************************
��������: ��������
�������: pFileWriter ����������CFileSeqWrite����ָ��, ����ִ�����,pFileWriter=NULL
���� :  ��. 
*********************************************************************/
TVOID CFileSeqWrite::Destroy(CFileSeqWrite*& pFileWriter)
{
	if ( NULL == pFileWriter)
	{
		return;
	}

	if ( 0 <= pFileWriter->m_hFile )
	{
		//�����ļ���δ�ر�
		pFileWriter->Close();
	}

	delete pFileWriter;

	pFileWriter = NULL;

	return;
}

/********************************************************************
��������: д�ļ�
�������: szSrcBuffer  Source bufferָ��
�������: udwWriteSize  Source�����ֽ���
���� :  ʵ��д�������ֽ���
*********************************************************************/
TINT32 CFileSeqWrite::Write(const TCHAR* szSrcBuffer, const TUINT32 udwWriteSize)
{
	if ( m_udwSizeOfBuff - m_udwUsedSizeOfBuff >= udwWriteSize )
	{
		//��ֱ��д��Buffer
		memcpy( m_szBuffer+m_udwUsedSizeOfBuff, szSrcBuffer, udwWriteSize);
		m_udwUsedSizeOfBuff += udwWriteSize;
		return (TINT32)udwWriteSize;
	}
	else
	{
		//�ӽ�Source��һ����д��Buffer
		TUINT32 udwSavedDest = (m_udwSizeOfBuff - m_udwUsedSizeOfBuff);		//�ѱ���Source�ֽ���
		memcpy(m_szBuffer+m_udwUsedSizeOfBuff, szSrcBuffer, udwSavedDest);
		//��Buffer��������д��Ӳ��
		if ( 0 > write(m_hFile, m_szBuffer, m_udwSizeOfBuff) )
		{
			//д�̳���
			return -1;
		}
		m_udwUsedSizeOfBuff = 0;
		//�ݹ����Write
		return  ( Write(szSrcBuffer+udwSavedDest,udwWriteSize-udwSavedDest) + udwSavedDest );
	}
	return -1;
}

/********************************************************************
��������: д�ļ�(���)
�������: szSrcBuffer  Source bufferָ��
�������: udwWriteSize  Source�����ֽ���
���� :  ʵ��д�������ֽ���
*********************************************************************/
TINT32 CFileSeqWrite::WriteHuge(const TCHAR* szSrcBuffer, const TUINT32 udwWritedSize)
{
	if ( 0 < m_udwUsedSizeOfBuff )
	{
		//���Ƚ�Buffer������д��
		if ( write(m_hFile,m_szBuffer,m_udwUsedSizeOfBuff) < 0 )
		{
			return -1;
		}
		m_udwUsedSizeOfBuff = 0;
	}
	//��Source������һ����д��
	TINT32 dwSavedSize = 0;
	if ( (dwSavedSize=write(m_hFile, szSrcBuffer, udwWritedSize)) < 0 )
	{
		return -1;
	}
	return dwSavedSize;
}

/********************************************************************
��������: ��һЩ������������д���ļ�
�������: ��Ӧ��������ָ��
���� :  ʵ��д�������ֽ���.
*********************************************************************/
/*
//INT32
TINT32 CFileSeqWrite::WriteINT32(const TINT32* pdwSrc)
{
	return Write((const TCHAR*)pdwSrc, m_udwINT32Size);
}

//INT64
TINT32 CFileSeqWrite::WriteINT64(const TINT64* pddwSrc)
{
	return Write((const TCHAR*)pddwSrc, m_udwINT64Size);
}

//UINT32
TINT32 CFileSeqWrite::WriteUINT32(const TUINT32* pudwSrc)
{
	return Write((const TCHAR*)pudwSrc, m_udwUINT32Size);
}

//UINT64
TINT32 CFileSeqWrite::WriteUINT64(const TUINT64* puddwSrc)
{
	return Write((const TCHAR*)puddwSrc, m_udwUINT64Size);
}

//Float32(float)
TINT32 CFileSeqWrite::WriteFloat32(const TFLOAT32* pfSrc)
{
	return Write((const TCHAR*)pfSrc, m_udwFloat32Size);
}

//Float64(double)
TINT32 CFileSeqWrite::WriteFloat64(const TFLOAT64* pfSrc)
{
	return Write((const TCHAR*)pfSrc, m_udwFloat64Size);
}

//String(char*)
TINT32 CFileSeqWrite::WriteString(const TCHAR* szSrc, const TUINT32 udwStrLen)
{
	return Write(szSrc, udwStrLen);
}

//String(char*)
TINT32 CFileSeqWrite::WriteString(const TCHAR* szSrc)
{
	return Write(szSrc, (TUINT32)strlen(szSrc));
}

//CHAR
TINT32 CFileSeqWrite::WriteCHAR(const TCHAR* pcSrc)
{
	return Write(pcSrc, m_udwCHARSize);
}

//INT8
TINT32 CFileSeqWrite::WriteINT8(const TINT8* pbySrc)
{
	return Write((const TCHAR*)pbySrc, m_udwINT8Size);
}

//INT16
TINT32 CFileSeqWrite::WriteINT16(const TINT16* pwSrc)
{
	return Write((const TCHAR*)pwSrc, m_udwINT16Size);
}

//UCHAR
TINT32 CFileSeqWrite::WriteUCHAR(const TUCHAR* pucSrc)
{
	return Write((const TCHAR*)pucSrc, m_udwUCHARSize);
}

//UINT8
TINT32 CFileSeqWrite::WriteUINT8(const TUINT8* pubySrc)
{
	return Write((const TCHAR*)pubySrc, m_udwUINT8Size);
}

//UINT16
TINT32 CFileSeqWrite::WriteUINT16(const TUINT16* puwSrc)
{
	return Write((const TCHAR*)puwSrc, m_udwUINT16Size);
}
*/

