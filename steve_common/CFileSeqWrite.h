/************************************************************
Copyright (C), 1998-2007, Tencent Technology Commpany Limited

文件名称: CFileSeqWrite.h
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

#ifndef __CFILESEQWRITE_H__
#define __CFILESEQWRITE_H__

#include "twsetypedef.h"
#include "string.h"

#define MAX_FILE_NAME_LENGTH_WRITE 500

/********************************************************************
功能描述: 文件顺序写操作类
主要对外接口: 
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
	功能描述: 创建对象
	输入参数: dwBuffSize 读文件buffer大小
	返回 :  非NULL, 新创建对象指针; NULL, 创建对象失败. 
	*********************************************************************/
	static CFileSeqWrite* Create(const TUINT32 udwBuffSize);

	/********************************************************************
	功能描述: 打开指定文件
	输入参数: szFileName  文件名
	输入参数: bToAppended, 若文件已存在是否在原文件后面追加, TRUE为追加, FALSE为覆盖, 默认是FALSE
	返回 :  打开文件是否成功, TRUE, 成功; FALSE,失败. 
	*********************************************************************/
	TBOOL Open(const TCHAR* szFileName, const TBOOL bToAppended = FALSE);

	/********************************************************************
	功能描述: 强制缓冲区内容写入硬盘文件
	输入参数: 无
	返回 :    TRUE,成功, FALSE,失败. 
	*********************************************************************/
	TBOOL Flush();

	/********************************************************************
	功能描述: 关闭文件
	输入参数: 无
	返回 :  无
	*********************************************************************/
	TVOID Close();

	/********************************************************************
	功能描述: 撤销对象
	输入参数: pFileWriter 即将撤销的CFileSeqWrite对象指针, 撤销执行完后,pFileWriter=NULL
	返回 :  无. 
	*********************************************************************/
	static TVOID Destroy(CFileSeqWrite*& pFileWriter);

	/********************************************************************
	功能描述: 写文件
	输入参数: szSrcBuffer  Source buffer指针
	输入参数: udwWriteSize  Source内容字节数
	返回 :  实际写盘内容字节数
	*********************************************************************/
	TINT32 Write(const TCHAR* szSrcBuffer, const TUINT32 udwWriteSize);

	/********************************************************************
	功能描述: 写文件(适用于大块内容写)
	输入参数: szSrcBuffer  Source buffer指针
	输入参数: udwWriteSize  Source内容字节数
	返回 :  实际写盘内容字节数
	*********************************************************************/
	TINT32 WriteHuge(const TCHAR* szDestBuffer, const TUINT32 udwReadSize);

	/********************************************************************
	功能描述: 从文件中读入一些常见数据类型
	输入参数: 相应数据类型指针
	返回 :  实际读出内容字节数.
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
	功能描述: 构造函数
	输入参数: 无
	返回 :  无
	*********************************************************************/
	CFileSeqWrite();

	/********************************************************************
	功能描述: 析构函数
	输入参数: 无
	返回 :  无
	*********************************************************************/
	~CFileSeqWrite();

	TCHAR * GetFileName() const { return (TCHAR *)m_szFileName; };

	TBOOL IsOpen() { return (m_hFile >= 0); };

private:
	//文件名
	TCHAR  m_szFileName[MAX_FILE_NAME_LENGTH_WRITE];

	//文件句柄
	TINT32 m_hFile;

	//Buffer指针
	TCHAR* m_szBuffer;

	//Buffer大小
	TUINT32 m_udwSizeOfBuff;

	//Buffer有效数据结束位置,m_dwUsedSizeOfBuff<=m_dwSizeOfBuff
	TUINT32 m_udwUsedSizeOfBuff;

	//Buffer已读数据结束位置,m_dwReadedSizeOfBuff<=m_udwSizeOfBuff
	//TUINT32 m_udwWritedSizeOfBuff;

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


#endif  //__CFILESEQWRITE_H__








