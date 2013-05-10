/************************************************************
Copyright (C), 1998-2007, Tencent Technology Commpany Limited

文件名称: CFileSeqRead.cpp
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

//一些常见数据类型Size初始赋值
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
功能描述: 构造函数
输入参数: 无
返回 :  无
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
功能描述: 析构函数
输入参数: 无
返回 :  无
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
功能描述: 创建对象
输入参数: dwBuffSize 读文件buffer大小
返回 :  非NULL, 新创建对象指针; NULL, 创建对象失败. 
*********************************************************************/
CFileSeqRead* CFileSeqRead::Create(const TUINT32 udwBuffSize)
{
	if ( 0 >= udwBuffSize )
		return NULL;

	//创建Buffer
	TCHAR* szBuffer = new TCHAR[udwBuffSize];
	if ( NULL == szBuffer )
		return NULL;

	//创建Reader对象
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
功能描述: 打开指定文件
输入参数: szFileName  文件名
返回 :  打开文件是否成功, TRUE, 成功; FALSE,失败. 
*********************************************************************/
TBOOL CFileSeqRead::Open(const TCHAR* szFileName)
{
	if ( 0 <= m_hFile )
	{
		//已打开文件尚未关闭
		return FALSE;
	}

	//以只读方式打开文件
#ifdef WIN32
	m_hFile = open( szFileName, O_RDONLY | O_BINARY | O_SEQUENTIAL, S_IREAD );
#else
	m_hFile = open( szFileName, O_RDONLY, S_IREAD );
#endif //WIN32
	if ( m_hFile == -1 )
	{
		//打开文件失败
		return FALSE;
	}
	//记录文件名
	strcpy( m_szFileName, szFileName );

	//初始化Buffer
	memset( m_szBuffer, 0, sizeof(TCHAR)*m_udwSizeOfBuff );

	//参数初始化
	m_udwUsedSizeOfBuff = 0;
	m_udwReadedSizeOfBuff = 0;

	//返回
	return TRUE;
}

/********************************************************************
功能描述: 关闭文件
输入参数: 无
返回 :  无
*********************************************************************/
TVOID CFileSeqRead::Close()
{
	//初始化Buffer
	memset( m_szBuffer, 0, sizeof(TCHAR)*m_udwSizeOfBuff );

	//参数初始化
	m_udwUsedSizeOfBuff = 0;
	m_udwReadedSizeOfBuff = 0;

	//关闭文件
	if ( 0 <= m_hFile )
	{
		close(m_hFile);
	}

	m_hFile = -1;

	return;
}

/********************************************************************
功能描述: 撤销对象
输入参数: pFileReader 读文件对象指针
返回 :  无. 
*********************************************************************/
TVOID CFileSeqRead::Destroy(CFileSeqRead*& pFileReader)
{
	if ( NULL == pFileReader)
	{
		return;
	}

	if ( 0 <= pFileReader->m_hFile )
	{
		//持有文件尚未关闭
		pFileReader->Close();
	}

	delete pFileReader;

	pFileReader = NULL;

	return;
}

/********************************************************************
功能描述: 读文件
输入参数: szDestBuffer 保存读出内容的目标buffer指针
输入参数: udwReadSize  读出内容大小
返回 :  实际读出内容大小.
*********************************************************************/
TINT32 CFileSeqRead::Read(TCHAR* szDestBuffer, const TUINT32 udwReadSize)
{
	if ( m_udwUsedSizeOfBuff - m_udwReadedSizeOfBuff >= udwReadSize )
	{
		//从已读Buffer中读取
		memcpy( szDestBuffer, (m_szBuffer+m_udwReadedSizeOfBuff), udwReadSize);
		m_udwReadedSizeOfBuff += udwReadSize;
		return (TINT32)udwReadSize;
	}
	else
	{
		//先从已读Buffer中读取一部分
		TUINT32 udwReadedDest = m_udwUsedSizeOfBuff-m_udwReadedSizeOfBuff;		//已读Buffer大小
		memcpy(szDestBuffer, (m_szBuffer+m_udwReadedSizeOfBuff), udwReadedDest);
		//从文件读入新一段
		m_udwReadedSizeOfBuff = 0;
		TINT32 dwNewReadFileSize = read(m_hFile, m_szBuffer, m_udwSizeOfBuff);
		if ( 0 >= dwNewReadFileSize )
		{
			//读到文件结尾或m_hFile无效或文件未打开或被锁定
			m_udwUsedSizeOfBuff = 0;
			return -1;
		}
		else
		{
			//成功读入新内容
			m_udwUsedSizeOfBuff = (TUINT32)dwNewReadFileSize;
		}
		
		//递归调用Read
		TINT32 dwIteraReadSize = Read(szDestBuffer+udwReadedDest,udwReadSize-udwReadedDest);
		if ( -1 == dwIteraReadSize )
		{
			//递归读文件出错
			return -1;
		}
		else
		{
			//递归读文件成功
			return  ( dwIteraReadSize + (TINT32)udwReadedDest );
		}
	}
	return -1;
}

/********************************************************************
功能描述: 读文件(大块)
输入参数: szDestBuffer 保存读出内容的目标buffer指针
输入参数: udwReadSize  读出内容大小
返回 :  实际读出内容大小.
*********************************************************************/
TINT32 CFileSeqRead::ReadHuge(TCHAR* szDestBuffer, const TUINT32 udwReadSize)
{
	if ( m_udwUsedSizeOfBuff - m_udwReadedSizeOfBuff >= udwReadSize )
	{
		//从已读Buffer中读取
		memcpy( szDestBuffer, (m_szBuffer+m_udwReadedSizeOfBuff), udwReadSize);
		m_udwReadedSizeOfBuff += udwReadSize;		//Buff中有效内容已读大小累加
		return ((TINT32)udwReadSize);
	}
	else
	{
		//先从已读Buffer中读取一部分
		TUINT32 udwReadedDest = m_udwUsedSizeOfBuff-m_udwReadedSizeOfBuff;		//已读Buffer大小
		memcpy(szDestBuffer, (m_szBuffer+m_udwReadedSizeOfBuff), udwReadedDest);
		m_udwReadedSizeOfBuff = 0;					//Buff中有效内容已读大小清零
		m_udwUsedSizeOfBuff = 0;					//Buff中有效内容大小清零
		//从文件将剩余部分一次读入目标Buffer
		TINT32 dwNewReadFileSize = read(m_hFile, szDestBuffer+udwReadedDest, udwReadSize-udwReadedDest);
		if ( dwNewReadFileSize < (TINT32)(udwReadSize-udwReadedDest) )
		{
			//读文件获得小于要求大小的内容
			return (-1);
		}
		else
		{
			//返回读入字节数
			return  ( dwNewReadFileSize + (TINT32)udwReadedDest );
		}
	}
	return -1;
}

/********************************************************************
功能描述: 从文件中读入一些常见数据类型
输入参数: 相应数据类型指针
返回 :  实际读出内容字节数.
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

