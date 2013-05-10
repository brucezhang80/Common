#ifndef __LIST_H__
#define __LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "_common.h"
	
/* 单链表节点 */
	
typedef struct slnode		
{
	struct slnode *next;
} SL_NODE;


/* 单链表头 */

typedef struct				
{
	SL_NODE *head;			
	SL_NODE *tail;			
} SL_LIST;

#define SLL_FIRST(pList)	(((SL_LIST *)(pList))->head)
#define SLL_LAST(pList)		(((SL_LIST *)(pList))->tail)
#define SLL_NEXT(pNode)		(((SL_NODE *)(pNode))->next)
#define SLL_EMPTY(pList) 	(((SL_LIST *)(pList))->head == NULL)

/* 函数声明 */

//extern SL_NODE *sll_each (SL_LIST *pList, FUNCPTR routine, int routineArg);
extern SL_NODE *sll_get (SL_LIST *pList);						/* 得到链表头节点，并取出 */
extern SL_NODE *sll_prev (SL_LIST *pList, SL_NODE *pNode);		/* 得到指定节点的前一节点 */
extern int 		sll_count (SL_LIST *pList);						/* 得到链表的长度 */
extern void 	sll_add_head(SL_LIST *pList, SL_NODE *pNode);	/* 在链表头部增加一节点 */
extern void 	sll_add_tail (SL_LIST *pList, SL_NODE *pNode);	/* 在链表尾部增加一节点 */
extern void 	sll_remove (SL_LIST *pList, SL_NODE *pDeleteNode, SL_NODE *pPrevNode); /* 删除指定节点 */

#define SLL_INIT(pList) do { \
	((SL_LIST *)(pList))->head = NULL; ((SL_LIST *)(pList))->tail = NULL; \
} while (0)
	
#define SLL_EACH(pList, pNode) \
	for (pNode = SLL_FIRST(pList); \
			pNode != NULL; \
			pNode = SLL_NEXT(pNode))

#define SLL_ENTRY(pNode, type, member) \
	((type *)((char *)(pNode) - (unsigned)(&((type *)0)->member)))

#if 0
/* 由于typeof为gcc扩展实现，windows下vc没有实现 */
#define SLL_EACH_ENTRY(pList, pEntry, member) \
	for (pEntry = SLL_ENTRY(SLL_FIRST(pList), typeof(*pEntry), member); \
		 &(pEntry->member) != NULL; \
		 pEntry = SLL_ENTRY(SLL_NEXT(&(pEntry->member)), typeof(*pEntry), member))
#endif
			
/* 双向链表节点(有两个指针) */
typedef struct dlnode
{
	struct dlnode *next;
	struct dlnode *prev;
} DL_NODE;

/* 双向链表头 */
typedef struct					
{
	DL_NODE *head;
	DL_NODE *tail;
} DL_LIST;

#define DLL_FIRST(pList) 	(((DL_LIST *)(pList))->head)
#define DLL_LAST(pList)		(((DL_LIST *)(pList))->tail)
#define DLL_NEXT(pNode)		(((DL_NODE *)(pNode))->next)
#define DLL_PREV(pNode)		(((DL_NODE *)(pNode))->prev)
#define DLL_EMPTY(pList) 	(((DL_LIST *)(pList))->head == NULL)

/* 双向链表函数声明 */

//extern DL_NODE *dll_each (DL_LIST *pList, FUNCPTR routine, int routineArg);
extern DL_NODE *dll_get (DL_LIST *pList);
extern int 		dll_count (DL_LIST *pList);

extern void 	dll_add (DL_LIST *pList, DL_NODE *pNode);						/* 尾部增加一个节点 */
extern void 	dll_insert (DL_LIST *pList, DL_NODE *pPrev, DL_NODE *pNode);	/* 在指定节点后插入一个节点*/
extern void 	dll_remove (DL_LIST *pList, DL_NODE *pNode);					/* 删除一个节点 */

#define DLL_INIT(pList) do { \
	((DL_LIST *)(pList))->head = NULL; ((DL_LIST *)(pList))->tail = NULL; \
} while (0)

#define DLL_ADD(pList, pNode) \
	dll_insert ((DL_LIST *)(pList), ((DL_LIST *)(pList))->tail, (DL_NODE *)(pNode))

#define DLL_EACH(pList, pNode) \
	for (pNode = DLL_FIRST(pList); \
			pNode != NULL; \
			pNode = DLL_NEXT(pNode))
#define DLL_EACH_BACK(pList, pNode) \
	for (pNode = DLL_LAST(pList); \
			pNode != NULL; \
			pNode = DLL_PREV(pNode))

#define DLL_ENTRY(pNode, type, member) \
	((type *)((char *)(pNode) - (unsigned)(&((type *)0)->member)))

#if 0
#define DLL_EACH_ENTRY(pList, pEntry,  member) \
	for (pEntry = DLL_ENTRY(DLL_FIRST(pList), typeof(*pEntry), member); \
		 &(pEntry->member) != NULL; \
		 pEntry = DLL_ENTRY(DLL_NEXT(&(pEntry->member)), typeof(*pEntry), member))
			
#define DLL_EACH_ENTRY_BACK(pList, pEntry,  member) \
	for (pEntry = DLL_ENTRY(DLL_LAST(pList), typeof(*pEntry), member); \
		 &(pEntry->member) != NULL; \
		 pEntry = DLL_ENTRY(DLL_PREV(&(pEntry->member)), typeof(*pEntry), member))
#endif

/* 绝对地址和相对地址的偏移 */

extern int local_to_global_Offset;

#define GLOB_TO_LOC_ADRS(adrs)	(((int)(adrs)) + local_to_global_Offset)
#define LOC_TO_GLOB_ADRS(adrs)	(((int)(adrs)) - local_to_global_Offset)
#define LOC_NULL				((void *)local_to_global_Offset)

/*  共享内存链表节点 */

typedef struct sm_dl_node	
{
	struct sm_dl_node * next;	 
	struct sm_dl_node * prev;	
} SM_NODE;

/*  共享内存链表头 */

typedef struct sm_dl_list
{
	SM_NODE * head;
	SM_NODE * tail;
} SM_LIST;

#define SM_DL_FIRST(pList)  (SM_NODE *)(GLOB_TO_LOC_ADRS ((int) (((SM_LIST *)(pList))->head)))
#define SM_DL_LAST(pList)   (SM_NODE *)(GLOB_TO_LOC_ADRS ((int) (((SM_LIST *)(pList))->tail)))
#define SM_DL_NEXT(pNode)   (SM_NODE *)(GLOB_TO_LOC_ADRS ((int) (((SM_NODE *)(pNode))->next)))
#define SM_DL_PREV(pNode)   (SM_NODE *)(GLOB_TO_LOC_ADRS ((int) (((SM_NODE *)(pNode))->prev)))
#define SM_DL_EMPTY(pList)  (((SM_LIST *)pList)->head == NULL)

//extern SM_NODE* 	smdll_each (SM_LIST * pList, FUNCPTR routine, int routineArg);
extern SM_NODE* 	smdll_get (SM_LIST * pList);
extern int         	smdll_count (SM_LIST * pList);
extern void        	smdll_add (SM_LIST * pList, SM_NODE * pNode);
extern void        	smdll_insert(SM_LIST * pList, SM_NODE * pPrev, SM_NODE * pNode);
extern void        	smdll_remove (SM_LIST * pList, SM_NODE * pNode);
extern void        	smdll_concat (SM_LIST * pDstList, SM_LIST * pAddList);

#define SMDLL_INIT(pList) do { \
	((SM_LIST *)(pList))->head = NULL; ((SM_LIST *)(pList))->tail = NULL;\
} while (0)

#define SMDLL_SETOFF(off) do {\
	local_to_global_Offset = (int)off;\
} while (0)

#define SMDLL_ADD(pList, pNode) \
		smdll_insert (pList, (SM_NODE *) GLOB_TO_LOC_ADRS (((int)((SM_LIST *)pList)->tail)), (SM_NODE *)pNode)

#define SMDLL_EACH(pList, pNode) \
	for (pNode = (SM_NODE *)SM_DL_FIRST(pList); \
			pNode != LOC_NULL; \
			pNode = (SM_NODE *)SM_DL_NEXT(pNode))
#define SMDLL_EACH_BACK(pList, pNode) \
	for (pNode = (SM_NODE *)DLL_LAST(pList); \
			pNode != LOC_NULL; \
			pNode = (SM_NODE *)SM_DL_PREV(pNode))

#define SMDLL_ENTRY(pNode, type, member) \
	((type *)((char *)(pNode) - (unsigned)(&((type *)0)->member)))

#ifdef __cplusplus
}
#endif

#endif
