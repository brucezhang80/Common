#ifndef __HASH_TABLE__H__
#define __HASH_TABLE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

/* ��ϣ��ͷ */

typedef struct hashtbl
{
	int		elements;		/* hash�ڵ���� */
	int 	nodenum;
	FUNCPTR	keyCmpRtn;		/* �ȽϺ��� */
	FUNCPTR	keyRtn;			/* hash���� */
	SL_LIST	*pHashTbl;		/* hash���� */
} HASH_TBL;

//typedef SL_NODE HASH_NODE;	/* ��ϣ�ڵ� */
typedef struct
{
	SL_NODE node;
	unsigned data[0];
}HASH_NODE;

typedef HASH_TBL *HASH_ID;

/* �������� */

// �����ɾ��hash��
extern HASH_ID 	hashtbl_create (int sizeLog2, FUNCPTR keyCmpRtn, FUNCPTR keyRtn);
extern void hashtbl_delete (HASH_ID hashId);

// ���Һͱ���hash��
 /*  BOOL routine (pNode, arg)
 *      Hash_Node *pNode;	 	�ڵ�ָ��   
 *      int	arg;	 			���� 
 */
extern HASH_NODE *hashtbl_each (HASH_ID hashId, FUNCPTR routine, int routineArg);
extern HASH_NODE *hashtbl_find (HASH_ID hashId, HASH_NODE *pMatchNode);

// ���ӣ�ɾ��һ��hash node
extern void hashtbl_add (HASH_ID hashId, HASH_NODE *pHashNode);
extern void hashtbl_remove (HASH_ID hashId, HASH_NODE *pHashNode);

// �ӽڵ��ҵ�����
#define HASH_ENTRY(pNode, type, member) \
	((type *)((char *)(pNode) - (unsigned long long)(&((type *)0)->member)))

// �����˼���������hash node
// �ַ�������
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
