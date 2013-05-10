#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 空指针 */
#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

/* TURE/FALSE定义 */	
#if	!defined(FALSE) || (FALSE!=0)
#define FALSE		0
#endif

#if	!defined(TRUE) || (TRUE!=1)
#define TRUE		1
#endif

/* OK ERROR定义 */
#define OK			0
#define ERROR		(-1)

#define MSB(x)	(((x) >> 8) & 0xff)	  /* 短整数的高字节 */
#define LSB(x)	((x) & 0xff)		  /* 短整数的低字节 */
#define MSW(x) (((x) >> 16) & 0xffff) /* 整数的高2字节 */
#define LSW(x) ((x) & 0xffff) 		  /* 整数低两字节 */

#define WORDSWAP(x) (MSW(x) | (LSW(x) << 16))	/* 整数的两个word交换 */

#define LLSB(x)	((x) & 0xff)		/* 整数低字节 */
#define LNLSB(x) (((x) >> 8) & 0xff)
#define LNMSB(x) (((x) >> 16) & 0xff)
#define LMSB(x)	 (((x) >> 24) & 0xff)

/* 整数的4个字节交换 */
#define LONGSWAP(x) ((LLSB(x) << 24) | \
				     (LNLSB(x) << 16)| \
				     (LNMSB(x) << 8) | \
				     (LMSB(x)))

/* 结构的某个成员的偏移量 */
#define OFFSET(structure, member)	\
			((int) &(((structure *) 0) -> member))

/* 某个成员大小 */
#define MEMBER_SIZE(structure, member)	\
			(sizeof (((structure *) 0) -> member))

/* 数组的元素个数 */ 
#define NELEMENTS(array)		\
			(sizeof (array) / sizeof ((array) [0]))

#define FOREVER	for (;;)

/* 两个数的最大值，最小值 */
#ifndef max
#define max(x, y)	(((x) < (y)) ? (y) : (x))
#define min(x, y)	(((x) < (y)) ? (x) : (y))
#endif
	
/* 声明 */
#define FAST	register
#define IMPORT	extern
#define LOCAL	static

/* 对齐(上对齐，下对齐，是否对齐) */
#define ROUND_UP(x, align)	(((int) (x) + (align - 1)) & ~(align - 1)) 
#define ROUND_DOWN(x, align)	((int)(x) & ~(align - 1))
#define ALIGNED(x, align)	(((int)(x) & (align - 1)) == 0)

/* 常见函数 */
typedef int (*FUNCPTR)(...);

#ifdef __cplusplus
}
#endif

#endif
