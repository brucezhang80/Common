/************************************************************
  Copyright (C), 1998-2006, Tencent Technology Commpany Limited

  �ļ�����: twsetypedef.h
  ����: swordshao       
  ����: 2006.12.12
  �汾: 1.0
  ģ������: ƽ̨�޹ص��������Ͷ���
            ÿ���������͵Ķ�����T��ͷ��T��ʾ����(Type)����˼��

  �޸���ʷ:         
      <author>    <time>   <version >   <desc>    
     swordshao  2006.12.12    1.0        ����
*************************************************************/

#ifndef __TWSE_TYPE_DEF_H__
#define __TWSE_TYPE_DEF_H__


typedef char                        TCHAR;
typedef unsigned char               TUCHAR;
typedef char                        TINT8;
typedef unsigned char               TUINT8;
typedef short                       TINT16;
typedef unsigned short              TUINT16;
typedef int                         TINT32;
typedef unsigned int                TUINT32;
typedef float                       TFLOAT32;
typedef double                      TFLOAT64;
typedef void                        TVOID;
typedef int                         TBOOL; 

#ifdef WIN32
    typedef __int64                 TINT64;
    typedef unsigned __int64        TUINT64;
#else
    #include <sys/types.h>
    typedef __int64_t               TINT64;
    typedef __uint64_t              TUINT64;
#endif


#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef NULL
#define NULL	0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL -1
#endif


#endif //__TWSE_TYPE_DEF_H__

