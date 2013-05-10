/************************************************************
Copyright (C), 1998-2007, Tencent Technology Commpany Limited

文件名称: CFileSeqRead.h
作者: glenliu
日期: 2007.1.29
版本: 1.0
模块描述: 文件顺序读操作类
组成类: 
CFileSeqRead
修改历史:
<author>    <time>   <version >   <desc>
glenliu   2007.1.29    1.0        创建
*************************************************************/
#ifndef __CFILESEQREAD_H__
#define __CFILESEQREAD_H__

#include "twsetypedef.h"


#define MAX_FILE_NAME_LENGTH_READ 500

/********************************************************************
功能描述: 文件顺序读操作类
主要对外接口: 
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
	功能描述: 创建对象
	输入参数: dwBuffSize 读文件buffer大小
	返回 :  非NULL, 新创建对象指针; NULL, 创建对象失败. 
	*********************************************************************/
	static CFileSeqRead* Create(const TUINT32 udwBuffSize);

	/********************************************************************
	功能描述: 打开指定文件
	输入参数: szFileName  文件名
	返回 :  打开文件是否成功, TRUE, 成功; FALSE,失败. 
	*********************************************************************/
	TBOOL Open(const TCHAR* szFileName);

	/********************************************************************
	功能描述: 关闭文件
	输入参数: 无
	返回 :  无
	*********************************************************************/
	TVOID Close();

	/********************************************************************
	功能描述: 撤销对象
	输入参数: pFileReader 即将撤销的CFileSeqRead对象指针, 撤销执行完后, pFileReader=NULL
	返回 :  无. 
	*********************************************************************/
	static TVOID Destroy(CFileSeqRead*& pFileReader);

	/********************************************************************
	功能描述: 读文件
	输入参数: szDestBuffer 保存读出内容的目标buffer指针
	输入参数: udwReadSize  读出内容大小
	返回 :  实际读出内容大小.
	*********************************************************************/
	TINT32 Read(TCHAR* szDestBuffer, const TUINT32 udwReadSize);

	/********************************************************************
	功能描述: 读文件(适用于大块内容读)
	输入参数: szDestBuffer 保存读出内容的目标buffer指针
	输入参数: udwReadSize  读出内容大小
	返回 :  实际读出内容大小.
	*********************************************************************/
	TINT32 ReadHuge(TCHAR* szDestBuffer, const TUINT32 udwReadSize);

	/********************************************************************
	功能描述: 从文件中读入一些常见数据类型
	输入参数: 相应数据类型指针
	返回 :  实际读出内容字节数.
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
	功能描述: 构造函数
	输入参数: 无
	返回 :  无
	*********************************************************************/
	CFileSeqRead();

	/********************************************************************
	功能描述: 析构函数
	输入参数: 无
	返回 :  无
	*********************************************************************/
	~CFileSeqRead();

	TCHAR * GetFileName() const { return (TCHAR *)m_szFileName; };

	TBOOL IsOpen() { return (m_hFile >= 0); };

private:
	//文件名
	TCHAR  m_szFileName[MAX_FILE_NAME_LENGTH_READ];

	//文件句柄
	TINT32 m_hFile;

	//Buffer指针
	TCHAR* m_szBuffer;

	//Buffer大小
	TUINT32 m_udwSizeOfBuff;

	//Buffer有效数据结束位置,m_dwUsedSizeOfBuff<=m_dwSizeOfBuff
	TUINT32 m_udwUsedSizeOfBuff;

	//Buffer已读数据结束位置,m_dwReadedSizeOfBuff<=m_dwUsedSizeOfBuff
	TUINT32 m_udwReadedSizeOfBuff;

	//一些常见数据类型Size
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


