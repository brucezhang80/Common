/************************************************************
Copyright (C), 1998-2007, Tencent Technology Commpany Limited

�ļ�����: CFileSeqWrite.h
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

#ifndef __CFILESEQWRITE_H__
#define __CFILESEQWRITE_H__

#include "twsetypedef.h"
#include "string.h"

#define MAX_FILE_NAME_LENGTH_WRITE 500

/********************************************************************
��������: �ļ�˳��д������
��Ҫ����ӿ�: 
Creat()
Open()
Write()
Close()
Destory()
*********************************************************************/
class CFileSeqWrite
{
public:
	/********************************************************************
	��������: ��������
	�������: dwBuffSize ���ļ�buffer��С
	���� :  ��NULL, �´�������ָ��; NULL, ��������ʧ��. 
	*********************************************************************/
	static CFileSeqWrite* Create(const TUINT32 udwBuffSize);

	/********************************************************************
	��������: ��ָ���ļ�
	�������: szFileName  �ļ���
	�������: bToAppended, ���ļ��Ѵ����Ƿ���ԭ�ļ�����׷��, TRUEΪ׷��, FALSEΪ����, Ĭ����FALSE
	���� :  ���ļ��Ƿ�ɹ�, TRUE, �ɹ�; FALSE,ʧ��. 
	*********************************************************************/
	TBOOL Open(const TCHAR* szFileName, const TBOOL bToAppended = FALSE);

	/********************************************************************
	��������: ǿ�ƻ���������д��Ӳ���ļ�
	�������: ��
	���� :    TRUE,�ɹ�, FALSE,ʧ��. 
	*********************************************************************/
	TBOOL Flush();

	/********************************************************************
	��������: �ر��ļ�
	�������: ��
	���� :  ��
	*********************************************************************/
	TVOID Close();

	/********************************************************************
	��������: ��������
	�������: pFileWriter ����������CFileSeqWrite����ָ��, ����ִ�����,pFileWriter=NULL
	���� :  ��. 
	*********************************************************************/
	static TVOID Destroy(CFileSeqWrite*& pFileWriter);

	/********************************************************************
	��������: д�ļ�
	�������: szSrcBuffer  Source bufferָ��
	�������: udwWriteSize  Source�����ֽ���
	���� :  ʵ��д�������ֽ���
	*********************************************************************/
	TINT32 Write(const TCHAR* szSrcBuffer, const TUINT32 udwWriteSize);

	/********************************************************************
	��������: д�ļ�(�����ڴ������д)
	�������: szSrcBuffer  Source bufferָ��
	�������: udwWriteSize  Source�����ֽ���
	���� :  ʵ��д�������ֽ���
	*********************************************************************/
	TINT32 WriteHuge(const TCHAR* szDestBuffer, const TUINT32 udwReadSize);

	/********************************************************************
	��������: ���ļ��ж���һЩ������������
	�������: ��Ӧ��������ָ��
	���� :  ʵ�ʶ��������ֽ���.
	*********************************************************************/

	//CHAR
	inline TINT32 WriteCHAR(const TCHAR* pcSrc)
	{
		return Write(pcSrc, m_udwCHARSize);
	}

	//INT8
	inline TINT32 WriteINT8(const TINT8* pbySrc)
	{
		return Write((const TCHAR*)pbySrc, m_udwINT8Size);
	}

	//INT16
	inline TINT32 WriteINT16(const TINT16* pwSrc)
	{
		return Write((const TCHAR*)pwSrc, m_udwINT16Size);
	}

	//INT32
	inline TINT32 WriteINT32(const TINT32* pdwSrc)
	{
		return Write((const TCHAR*)pdwSrc, m_udwINT32Size);
	}

	//INT64
	inline TINT32 WriteINT64(const TINT64* pddwSrc)
	{
		return Write((const TCHAR*)pddwSrc, m_udwINT64Size);
	}

	//UCHAR
	inline TINT32 WriteUCHAR(const TUCHAR* pucSrc)
	{
		return Write((const TCHAR*)pucSrc, m_udwUCHARSize);
	}

	//UINT8
	inline TINT32 WriteUINT8(const TUINT8* pubySrc)
	{
		return Write((const TCHAR*)pubySrc, m_udwUINT8Size);
	}

	//UINT16
	inline TINT32 WriteUINT16(const TUINT16* puwSrc)
	{
		return Write((const TCHAR*)puwSrc, m_udwUINT16Size);
	}

	//UINT32
	inline TINT32 WriteUINT32(const TUINT32* pudwSrc)
	{
		return Write((const TCHAR*)pudwSrc, m_udwUINT32Size);
	}

	//UINT64
	inline TINT32 WriteUINT64(const TUINT64* puddwSrc)
	{
		return Write((const TCHAR*)puddwSrc, m_udwUINT64Size);
	}

	//Float32(float)
	inline TINT32 WriteFloat32(const TFLOAT32* pfSrc)
	{
		return Write((const TCHAR*)pfSrc, m_udwFloat32Size);
	}

	//Float64(double)
	inline TINT32 WriteFloat64(const TFLOAT64* pfSrc)
	{
		return Write((const TCHAR*)pfSrc, m_udwFloat64Size);
	}

	//String(char*)
	inline TINT32 WriteString(const TCHAR* szSrc, const TUINT32 udwStrLen)
	{
		return Write(szSrc, udwStrLen);
	}

	//String(char*)
	inline TINT32 WriteString(const TCHAR* szSrc)
	{
		return Write(szSrc, (TUINT32)strlen(szSrc));
	}

	/********************************************************************
	��������: ���캯��
	�������: ��
	���� :  ��
	*********************************************************************/
	CFileSeqWrite();

	/********************************************************************
	��������: ��������
	�������: ��
	���� :  ��
	*********************************************************************/
	~CFileSeqWrite();

	TCHAR * GetFileName() const { return (TCHAR *)m_szFileName; };

	TBOOL IsOpen() { return (m_hFile >= 0); };

private:
	//�ļ���
	TCHAR  m_szFileName[MAX_FILE_NAME_LENGTH_WRITE];

	//�ļ����
	TINT32 m_hFile;

	//Bufferָ��
	TCHAR* m_szBuffer;

	//Buffer��С
	TUINT32 m_udwSizeOfBuff;

	//Buffer��Ч���ݽ���λ��,m_dwUsedSizeOfBuff<=m_dwSizeOfBuff
	TUINT32 m_udwUsedSizeOfBuff;

	//Buffer�Ѷ����ݽ���λ��,m_dwReadedSizeOfBuff<=m_udwSizeOfBuff
	//TUINT32 m_udwWritedSizeOfBuff;

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


#endif  //__CFILESEQWRITE_H__








