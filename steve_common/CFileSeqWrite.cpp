/************************************************************
Copyright (C), 1998-2007, Tencent Technology Commpany Limited

文件名称: CFileSeqWrite.cpp
作者: glenliu
日期: 2007.1.29
版本: 1.0
模块描述: 文件顺序写操作类
组成类: 
CFileSeqWrite
修改历史:
<author>    <time>   <version >   <desc>
glenliu   2007.1.29    1.0        创建
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

//一些常见数据类型Size初始赋值
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
功能描述: 构造函数
输入参数: 无
返回 :  无
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
功能描述: 析构函数
输入参数: 无
返回 :  无
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
功能描述: 创建对象
输入参数: dwBuffSize 读文件buffer大小
返回 :  非NULL, 新创建对象指针; NULL, 创建对象失败. 
*********************************************************************/
CFileSeqWrite* CFileSeqWrite::Create(const TUINT32 udwBuffSize)
{
	if ( 0 >= udwBuffSize )
		return NULL;

	//创建Buffer
	TCHAR* szBuffer = new TCHAR[udwBuffSize];
	if ( NULL == szBuffer )
		return NULL;

	//创建Reader对象
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
功能描述: 打开指定文件
输入参数: szFileName  文件名
返回 :  打开文件是否成功, TRUE, 成功; FALSE,失败. 
*********************************************************************/
TBOOL CFileSeqWrite::Open(const TCHAR* szFileName, const TBOOL bToAppended)
{
	if ( 0 <= m_hFile )
	{
		//已打开文件尚未关闭
		return FALSE;
	}

	//以只写方式打开文件
	if ( bToAppended )
	{
		//追加方式
#ifdef WIN32
		m_hFile = open( szFileName, O_APPEND | O_CREAT | O_WRONLY | O_BINARY | O_SEQUENTIAL, S_IWRITE);
#else
		m_hFile = open( szFileName, O_APPEND | O_CREAT | O_WRONLY, 0666 );
#endif //WIN32
	}
	else
	{
		//覆盖方式
#ifdef WIN32
		m_hFile = open( szFileName, O_TRUNC | O_CREAT | O_WRONLY | O_BINARY | O_SEQUENTIAL, S_IWRITE);//, S_IWRITE
#else
		m_hFile = open( szFileName, O_TRUNC | O_CREAT | O_WRONLY, 0666 );//, S_IWRITE
#endif
	}
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

	//返回
	return TRUE;
}
/********************************************************************
功能描述: 强制将缓冲区内容写入硬盘文件
输入参数: 无
返回 :    TRUE,成功, FALSE,失败. 
*********************************************************************/
TBOOL CFileSeqWrite::Flush()
{
	if ( 0 > m_hFile )
	{
		return FALSE;
	}

	//Buffer内容写盘
	if ( 0 > write(m_hFile, m_szBuffer, m_udwUsedSizeOfBuff) )
	{
		//写盘出错
		m_udwUsedSizeOfBuff = 0;
		return FALSE;
	}
	//参数初始化
	m_udwUsedSizeOfBuff = 0;
	return TRUE;
}

/********************************************************************
功能描述: 关闭文件
输入参数: 无
返回 :  无
*********************************************************************/
TVOID CFileSeqWrite::Close()
{
	if ( 0 <= m_hFile )
	{
		//Buffer内容写盘
		write(m_hFile, m_szBuffer, m_udwUsedSizeOfBuff);

		//参数初始化
		m_udwUsedSizeOfBuff = 0;

		close(m_hFile);
	}
	m_hFile = -1;

	return;
}

/********************************************************************
功能描述: 撤销对象
输入参数: pFileWriter 即将撤销的CFileSeqWrite对象指针, 撤销执行完后,pFileWriter=NULL
返回 :  无. 
*********************************************************************/
TVOID CFileSeqWrite::Destroy(CFileSeqWrite*& pFileWriter)
{
	if ( NULL == pFileWriter)
	{
		return;
	}

	if ( 0 <= pFileWriter->m_hFile )
	{
		//持有文件尚未关闭
		pFileWriter->Close();
	}

	delete pFileWriter;

	pFileWriter = NULL;

	return;
}

/********************************************************************
功能描述: 写文件
输入参数: szSrcBuffer  Source buffer指针
输入参数: udwWriteSize  Source内容字节数
返回 :  实际写盘内容字节数
*********************************************************************/
TINT32 CFileSeqWrite::Write(const TCHAR* szSrcBuffer, const TUINT32 udwWriteSize)
{
	if ( m_udwSizeOfBuff - m_udwUsedSizeOfBuff >= udwWriteSize )
	{
		//可直接写入Buffer
		memcpy( m_szBuffer+m_udwUsedSizeOfBuff, szSrcBuffer, udwWriteSize);
		m_udwUsedSizeOfBuff += udwWriteSize;
		return (TINT32)udwWriteSize;
	}
	else
	{
		//从将Source中一部分写入Buffer
		TUINT32 udwSavedDest = (m_udwSizeOfBuff - m_udwUsedSizeOfBuff);		//已保存Source字节数
		memcpy(m_szBuffer+m_udwUsedSizeOfBuff, szSrcBuffer, udwSavedDest);
		//将Buffer中内容先写入硬盘
		if ( 0 > write(m_hFile, m_szBuffer, m_udwSizeOfBuff) )
		{
			//写盘出错
			return -1;
		}
		m_udwUsedSizeOfBuff = 0;
		//递归调用Write
		return  ( Write(szSrcBuffer+udwSavedDest,udwWriteSize-udwSavedDest) + udwSavedDest );
	}
	return -1;
}

/********************************************************************
功能描述: 写文件(大块)
输入参数: szSrcBuffer  Source buffer指针
输入参数: udwWriteSize  Source内容字节数
返回 :  实际写盘内容字节数
*********************************************************************/
TINT32 CFileSeqWrite::WriteHuge(const TCHAR* szSrcBuffer, const TUINT32 udwWritedSize)
{
	if ( 0 < m_udwUsedSizeOfBuff )
	{
		//首先将Buffer中内容写盘
		if ( write(m_hFile,m_szBuffer,m_udwUsedSizeOfBuff) < 0 )
		{
			return -1;
		}
		m_udwUsedSizeOfBuff = 0;
	}
	//将Source中内容一次性写盘
	TINT32 dwSavedSize = 0;
	if ( (dwSavedSize=write(m_hFile, szSrcBuffer, udwWritedSize)) < 0 )
	{
		return -1;
	}
	return dwSavedSize;
}

/********************************************************************
功能描述: 将一些常见数据类型写入文件
输入参数: 相应数据类型指针
返回 :  实际写入内容字节数.
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

