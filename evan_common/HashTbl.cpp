     
#include <stdlib.h>
#include <string.h>
#include "HashTbl.h"

// 初始化hash表头部，并初始化hash slot的链表头
static void hashtbl_init (HASH_TBL *pHashTbl, SL_LIST *pTblMem, int sizeLog2, FUNCPTR keyCmpRtn,  FUNCPTR keyRtn)         
{
	register int ix;

	/* 初始化hash头 */
	pHashTbl->elements	= 1 << sizeLog2;	
	pHashTbl->nodenum = 0;
	pHashTbl->keyCmpRtn	= keyCmpRtn;		
	pHashTbl->keyRtn	= keyRtn;			
	pHashTbl->pHashTbl	= pTblMem;

	/* 初始化hash链表头 */

	for (ix = 0; ix < pHashTbl->elements; ix++)
		SLL_INIT(&pHashTbl->pHashTbl[ix]);
}

// 创建hash表，其中元素个数为(1<<sizelog2)个，需要提供比较函数，和hash函数
HASH_ID hashtbl_create (int sizeLog2, /* 元素个数 = (1 << sizelog2) */
 				FUNCPTR keyCmpRtn,    /* 节点比较函数 */
 				FUNCPTR keyRtn       /* hash函数 */)
{
	unsigned extra  = (1 << sizeLog2) * sizeof (SL_LIST);
	HASH_ID hashId;
	SL_LIST *pList;

	/* 创建hash表 */
	hashId = (HASH_ID)malloc(sizeof(HASH_TBL) + extra);
	if (hashId == NULL)
		return NULL;

	pList = (SL_LIST *)((char *)hashId + sizeof(HASH_TBL));

	/* 初始化hash表 */
	if (hashId != NULL)
		hashtbl_init (hashId, pList, sizeLog2, keyCmpRtn, keyRtn);

	return hashId;
}

// 删除hash表，主要是删除hash slot部分，hash node部分是用户来控制的
void hashtbl_delete (HASH_ID hashId)
{
	if (hashId)
		free(hashId);
		
	hashId = NULL;
}

// 添加一个hash node到hash表中
void hashtbl_add (HASH_ID hashId, HASH_NODE *pHashNode)
{
	unsigned	index;

	/* 计算新加入节点的hash值 */
	index = (*hashId->keyRtn)(hashId->elements, pHashNode);

	/* 加入到hash链表中 */
	sll_add_head (&hashId->pHashTbl[index], &pHashNode->node);
	hashId->nodenum++;
}

// 查找该hash node是否在hash表中
HASH_NODE *hashtbl_find (FAST HASH_ID hashId, HASH_NODE *pMatchNode)
{
	register HASH_NODE *pHNode;
	int	 ix;

	/* 通过key计算hash值 */
	ix = (*hashId->keyRtn) (hashId->elements, pMatchNode);
	pHNode = HASH_ENTRY(SLL_FIRST (&hashId->pHashTbl [ix]), HASH_NODE, node);

	/* 比较key, 顺序遍历链表 */
	while ((pHNode != NULL) &&
			!((* hashId->keyCmpRtn) (pMatchNode, pHNode)))
		pHNode = HASH_ENTRY(SLL_NEXT (&pHNode->node), HASH_NODE, node);

	return pHNode;
}

// 从hash表中删除一个hash node
void hashtbl_remove (HASH_ID  hashId, HASH_NODE *pHashNode)
{
	HASH_NODE *pPrevNode;
	int	ix;

	ix = (* hashId->keyRtn) (hashId->elements, pHashNode);

	/* 得到指定节点的前一节点 */
	pPrevNode = HASH_ENTRY(sll_prev (&hashId->pHashTbl[ix], &pHashNode->node), HASH_NODE, node);

	/* 删除指定节点 */
	sll_remove (&hashId->pHashTbl[ix], &pHashNode->node, &pPrevNode->node);
	hashId->nodenum--;
}

#if 0
// 遍历hash表
HASH_NODE *hashtbl_each (HASH_ID hashId, FUNCPTR routine, int routineArg)
{
	FAST int  ix;
	HASH_NODE *pNode = NULL;

	for (ix = 0; (ix < hashId->elements) && (pNode == NULL); ix++)
		pNode = (HASH_NODE *)sll_each (&hashId->pHashTbl[ix],routine,routineArg);

	// 返回停止的节点
	return (pNode);	
}
#endif

// 字符串做key的时候对应的hash函数 
int hashfunc_string (int  elements,  H_NODE_STRING *pHNode)
{
	#define SEED 13 				/* 经验：13或者27作为种子较好 */

	register char	*tkey;
	register int	hash = 0;

	/* 计算hash值 */

	for (tkey = pHNode->string; *tkey != '\0'; tkey++)
		hash = hash * SEED + (unsigned int) *tkey; // 一次取出每个字符hash

	return (hash & (elements - 1));	/* 使hash值落在[0,elements) */
}

// 数值类型做key的时候比较函数
int hashkey_cmp (H_NODE_INT  *pMatchHNode, H_NODE_INT  *pHNode)
{
	return 	(pMatchHNode->key == pHNode->key);
}

// 字符串做key的时候比较函数
int hashkey_strcmp (H_NODE_STRING *pMatchHNode, H_NODE_STRING *pHNode)
{
	return 	strcmp (pMatchHNode->string, pHNode->string);
}
