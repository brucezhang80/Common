
#include "list.h"

/* 在头部插入一节点 */
void sll_add_head (SL_LIST *pList, SL_NODE *pNode)
{
	/* 如果链表为空 */
	if ((pNode->next = pList->head) == NULL)
		pList->head = pList->tail = pNode;
	else
		pList->head = pNode;
}

/* 在尾部插入一节点 */
void sll_add_tail (SL_LIST *pList, SL_NODE *pNode)
{
	pNode->next = NULL;

	/* 如果链表为空 */
	if (pList->head == NULL)
		pList->tail = pList->head = pNode;
	else
		pList->tail->next = pNode;
	pList->tail = pNode;
}

/* 得到首节点，并删除 */
SL_NODE *sll_get (register SL_LIST *pList)
{
	register SL_NODE *pNode;

	if ((pNode = pList->head) != NULL)
		pList->head = pNode->next;

	return (pNode);
}

/* 删除指定节点 */
void sll_remove (SL_LIST *pList, SL_NODE *pDeleteNode, SL_NODE *pPrevNode)
{
	/* 删除首节点 */
	if (pPrevNode == NULL)
	{
		pList->head = pDeleteNode->next;
		if (pList->tail == pDeleteNode)
			pList->tail = NULL;
	}
	else
	{
		pPrevNode->next = pDeleteNode->next;
		if (pList->tail == pDeleteNode)
			pList->tail = pPrevNode;
	}
}

/* 找到指定节点的前一节点 */
SL_NODE *sll_prev (SL_LIST *pList, SL_NODE *pNode)
{
	SL_NODE *pTmpNode = pList->head;

	/* 如果链表为空，或者是首节点 */
	if ((pTmpNode == NULL) || (pTmpNode == pNode))
		return (NULL);					

	/* 依次遍历 */
	while (pTmpNode->next != NULL)
	{
		if (pTmpNode->next == pNode)
			return (pTmpNode);

		pTmpNode = pTmpNode->next;
	}

	return (NULL);					
}

/* 求链表的节点个数 */
int sll_count (SL_LIST *pList)
{
	register SL_NODE *pNode = SLL_FIRST (pList);
	register int count = 0;

	while (pNode != NULL)
	{
		count ++;
		pNode = SLL_NEXT (pNode);
	}

	return (count);
}

/* 依次对每个节点执行某个函数，如果函数返回FALSE，停止，并返回该节点 */
SL_NODE *sll_each (SL_LIST *pList, FUNCPTR routine, int routineArg)
{
	register SL_NODE *pNode = SLL_FIRST (pList);
	register SL_NODE *pNext;

	while (pNode != NULL)
	{
		pNext = SLL_NEXT (pNode);
		if ((* routine) (pNode, routineArg) == FALSE)
			break;
		pNode = pNext;
	}

	return (pNode);	
}

