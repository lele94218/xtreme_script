/**
 * Author: Taoran Xue
 * Date Created: 1/5/2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SOURCE_LINE_SIZE 4096


char ** g_ppstrSourceCode = NULL;
int g_iSourceCodeSize;

typedef struct _Instr
{
	int iOpcode;
	int iOpCount;
	Op * pOpList;
}
	Instr;

typedef struct _Op
{
	int iType;
	union
	{
		int iIntLiteral;
		float fFloatLiteral;
		int iStringTableIndex;
		int iStackIndex;
		int iInstrIndex;
		int iFuncIndex;
		int iHostAPICallIndex;
		int iReg;
	};
	int iOffsetIndex;
}
	Op;

Instr * g_pInstrStream = NULL;
int g_iInstrStreamSize;

// Script header data, data
typedef struct _ScriptHeader
{
	int iStackSize;
	int iGlobalDataSize;
	int iIsMainFuncPresent;
	int iMainFuncIndex;
}
	ScriptHeader;

typedef struct _LinkedListNode
{
	void * pData;
	_LinkedListNode * pNext;
}
	LinkedListNode;

typedef struct _LinkedList
{
	LinkedListNode *pHead,
				   *pTail;
	int iNodeCount;
}
	LinkedList;

void InitLinkedList ( LinkedList * pList )
{
	pList->pHead = NULL;
	pList->pTail = NULL;

	pList->iNodeCount = 0;
}

int AddNode ( LinkedList * pList, void * pData )
{
	LinkedListNode * pNewNode = ( LinkedListNode * )
		malloc (sizeof ( LinkedListNode ));

	pNewNode->pData = pData;
	pNewNode->pNext = NULL;

	if ( ! pList->iNodeCount)
	{
		pList->pHead = pNewNode;
		pList->pTail = pNewNode;
	}

	else
	{
		pList->pTail->pNext = pNewNode;
		pList->pTail = pNewNode;
	}

	++pList->iNodeCount;

	return pList->iNodeCount - 1;
}

void FreeLinkedList ( LinkedList * pList )
{
	if ( ! pList )
		return;

	if ( pList->iNodeCount )
	{
		LinkedListNode * pCurrNode,
					   * pNextNode;
		
		pCurrNode = pList->pHead;

		while ( true )
		{
			pNextNode = pCurrNode->pNext;

			if (  pCurrNode->pData)
				free ( pCurrNode->pData );

			if ( pCurrNode )
				free ( pCurrNode);

			if ( pNextNode)
				pCurrNode = pNextNode;
			else
				break;
		}
	}
}


