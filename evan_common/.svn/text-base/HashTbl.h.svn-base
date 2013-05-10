#ifndef __HASH_TABLE__H__
#define __HASH_TABLE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

/* 哈希表头 */

typedef struct hashtbl
{
	int		elements;		/* hash节点个数 */
	int 	nodenum;
	FUNCPTR	keyCmpRtn;		/* 比较函数 */
	FUNCPTR	keyRtn;			/* hash函数 */
	SL_LIST	*pHashTbl;		/* hash链表 */
} HASH_TBL;

//typedef SL_NODE HASH_NODE;	/* 哈希节点 */
typedef struct
{
	SL_NODE node;
	unsigned data[0];
}HASH_NODE;

typedef HASH_TBL *HASH_ID;

/* 函数声明 */

// 创建蜕境齢ash表
extern HASH_ID 	hashtbl_create (int sizeLog2, FUNCPTR keyCmpRtn, FUNCPTR keyRtn);
extern void hashtbl_delete (HASH_ID hashId);

// 查找和遍历hash表
 /*  BOOL routine (pNode, arg)
 *      Hash_Node *pNode;	 	节点指针   
 *      int	arg;	 			参数 
 */
extern HASH_NODE *hashtbl_each (HASH_ID hashId, FUNCPTR routine, int routineArg);
extern HASH_NODE *hashtbl_find (HASH_ID hashId, HASH_NODE *pMatchNode);

// 增加，删除一个hash node
extern void hashtbl_add (HASH_ID hashId, HASH_NODE *pHashNode);
extern void hashtbl_remove (HASH_ID hashId, HASH_NODE *pHashNode);

// 从节点找到内容
#define HASH_ENTRY(pNode, type, member) \
	((type *)((char *)(pNode) - (unsigned long long)(&((type *)0)->member)))

// 定义了几个常见的hash node
// 字符串类型
typedef struct 
{
	HASH_NODE node;
	char *string;
	int data;
}H_NODE_STRING;

typedef struct
{
	HASH_NODE node;
	int key;
	int data;
}H_NODE_INT;

#ifdef __cplusplus
}
#endif

#endif
