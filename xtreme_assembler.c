/**
 * Author: Taoran Xue
 * Date Created: 1/5/2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SOURCE_LINE_SIZE 4096
#define MAX_INSTR_LOOKUP_COUNT 256
#define	MAX_INSTR_MNEMONIC_SIZE 16


// ---- Data Structures ----

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

typedef struct _FuncNode
{
	int iIndex;
	char pstrName [ MAX_IDENT_SIZE ];
	int iEntryPoint;
	int iParamCount;
	int iLocalDataSize
}
	FuncNode;

typedef struct _LabelNode
{
	int iIndex;
	char pastrIndent [ MAX_INDENT_SIZE ];
	int iTargetIndex;
	int iFuncIndex;
}

typedef int OpTypes;

typedef struct _InstrLookup
{
	char pstrMnemonic [ MAX_INSTR_MNEMONIC_SIZE ];
	int iOpcode;
	int iOpCount;
	OpTypes * OpList;
}
	InstrLookup;

typedef struct _SymbolNode
{
	int Index;
	char pstrIdent [ MAX_IDENT_SIZE ];
	int iSize;
	int iStackIndex;
	int iFuncIndex;
}
	SymbolNode;


// ---- Global Variables ----

// Source code representation
char ** g_ppstrSourceCode = NULL;
int g_iSourceCodeSize;

// The instruction lookup table
InstrLookup g_InstrTable [ MAX_INSTR_LOOKUP_COUNT ]

// The assembled instruction stream
Instr * g_pInstrStream = NULL;
int g_iInstrStreamSize;

// The script header
ScriptHeader g_ScriptHeader;

// The main tables
LinkedList g_StringTable;
LinkedList g_FuncTable;
LinkedList g_SymbolTable;
LinkedList g_LabelTable;
LinkedList g_HostAPICallTable;

// ---- Function declarations ----
int AddString ( LinkedList * pList, char * pstrString );
int AddFunc ( char * pstrName, int iEntryPoint );
void SetFuncInfo ( char * pstrName, int iParamCount, int iLocalDataSize );

int AddSymbol ( char * pstrIdent, int iSize, int iStackIndex, int iFuncIndex );
SymbolNode * GetSymbolByIdent ( char * pstrIdent, int iFuncIndex );
int GetStackIndexByIdent ( char * pstrIndent, int iFuncIndex );
int GetSizeByIdent ( char * pstrIdent, int iFuncIndex );

int AddLabel ( char * pstrIdent, int iTargetIndex, int iFuncIndex );
LabelNode * GetLabelByIdent ( char * pstrIdent, int iFuncIndex );

int AddInstrLookup ( char * pstrMnemonic, int iOpcode, int iOpCount );
void SetOpType ( int iInstrIndex, int iOpIndex, OpTypes iOpType );
int GetInstrByMnemonic ( char * pstrMnemonic, InstrLookup * pInstr );


// ---- Functions ----

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

int AddString( LinkedList * pLIst, char * pstrString )
{
	LinkedListNode * pNode = pList->pHead;

	for ( int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++ iCurrNode )
	{
		if ( strCmp ( ( char * ) pNode->pData, pstrString ) == 0 )
			return iCurrNode;
		
		pNode = pNode->pNext;
	}

	char * pstrStringNode = (char *) malloc (strlen ( pstrString ) + 1);
	strcpy ( pstrStringNode, pstrString );

	return AddNode ( pList, pstrStringNode );
}

int AddFunc (char * pStrName, int iEntryPoint)
{
	if ( GetFuncByName ( pstrName ) )
		return -1;

	FuncNode * pNewFunc = ( FuncNode * ) malloc ( sizeof ( FuncNode ) );

	strcpy ( pNewFunc->pstrName, pstrName);
	pNewFunc->iEntryPoint = iEntryPoint;

	int iIndex = AddNode ( & g_FuncTable, pNewFunc );

	pNewFunc->iIndex = iIndex;

	return iIndex;
}

void SetFuncInfo ( char * pstrName, int iParamCount, int iLocalDataSize )
{
	FuncNode * pFunc GetFuncByName ( pstrName );

	pFunc->iParamCount = iParamCount;
	pFunc->iLocalDataSize = iLocalDataSize;
}

FuncNode * GetFuncByName ( char * pstrName )
{
	if ( ! g_FuncTable.iNodeCount )
		return NULL:
	
	LinkedListNode * pCurrNode = g_FUncTable.pHead;
	
	for (int iCurrNode = 0; iCurrNode < g_FuncTable.iNodeCount; ++ iCurrNode)
	{
		FuncNode * pCurrFunc = ( FuncNode * ) pCurrNode->pData;

		if ( strcmp ( pCurrFunc->pstrName, pstrName ) == 0 )
			return pCurrFunc;

		pCurrNode = pCurrNode->pNext;
	}
	return NULL;
}

int AddSymbol ( char * pstrIdent, int iSize, int iStackIndex int iFuncIndex )
{
	if ( GetSymbolByIdent ( pstrIdent, iFuncIndex))
		return -1;

	SymbolNode * pNewSymbol = ( SymbolNode * )
		malloc ( sizeof ( SymbolNode ) );

	strcpy ( pNewSymbol->pstrIdent, pstrIdent );
	pNewSymbol->iSize = iSize;
	pNewSymbol->iStackIndex = iStackIndex;
	pNewSymbol->iFuncIndex = iFuncIndex;

	int iIndex = AddNode ( & g_SymbolTable, pNewSymbol );

	pNewSymbol->iIndex = iIndex;

	return iIndex;
}

SymbolNode * GetSymbolByIdent ( char * pstrIndent, int iFuncIndex )
{
	if ( ! g_SymbolTable.iNodeCount )
		return NULL;
	
	LinkedListNode * pCurrNode = g_SymbolTable.pHead;

	for ( int iCurrNode = 0; iCurrNode < g_SymbolTable.iNodeCount; ++ iCurrNode )
	{
		SymbolNode * pCurrSymbol = ( SymbolNode * ) pCurrNode->pData;

		if ( strcmp ( pCurrSymbol->pstrIdent, pstrIdent) == 0 )
			if ( pCurrSymbol->iFuncIndex == iFuncIndex 
					|| pCurrSymbol->iStackIndex >= 0 )
				return pCurrSymbol

		pCurrNode = pCurrNode->pNext;
	}

	return NULL;
}

int GetStackIndexByIndent ( char * pstrIndent, int iFuncIndex )
{
	SymbolNode * pSymbol = GetSymbolByIndent ( pstrIndent, iFuncIndex );
	
	return pSymbol->iStackIndex;
}

int GetSizeByIndent ( char * pstrIdent, int IFuncIndex )
{
	SymbolNode * pSymbol = GetSymbolByIndent ( pstrIent, iFuncIndex );

	return pSymbol->iSize;
}

int AddLabel( char * pstrIdent, int iTargetIndex, int iFuncIndex)
{
	if ( GetLabelByIndent ( pstrIdent, iFuncIndex) )
		return -1;

	LabelNode * pNewLabel = (Label* ) malloc ( sizeof ( LabelNode) );

	strcpy ( pNewLabel->pstrIdent, pstrIdent );
	pNewLabel->iTargetIndex = iTargetIndex;
	pNewLabel->iFuncIndex = iFuncIndex;

	int iIndex = AddNode ( & g_LabelTable, pNewLabel );

	pNewLabel->iIndex = iIndex;

	return iIndex;
}

LabelNode * GetLabelByIndex ( char * pstrIdent, int iFuncIndex )
{
	if (! g_LabelTable.iNodeCount)
		return NULL;

	LinkedListNode * pCurrNode = g_LabelTable.pHead;

	for ( int iCurrNode = 0; iCurrNode < g_LabelTable.iNodeCount; ++ iCurrNode )
	{
		LabelNode * pCurrLabel = ( LabelNode * ) pCurrNode -> pData;

		if ( strcmp ( pCurrLabel->pstrIdent, pstrIdent ) == 0
				&& pCurrLabel->iFuncIndex == iFuncIndex )
			return pCurrLabel;

		pCurrNOde = pCurrNode->pNext;
	}

	return NULL;
}

int AddInstrLookup ( char * pstrMnemonic, int iOpcode, int iOpCount )
{
	static int iInstrIndex = 0;

	if ( iInstrIndex >= MAX_INSTR_LOOKUP_COUNT )
		return -1;

	strcpy ( g_InstrTable [ iInstrIndex ].pstrMnemonic, pstrMnemotic );
	strupr ( g_InstrTable [ iInstrIndex ] );
	g_InstrTable [ iInstrIndex ].iOpcode = iOpcode;
	g_InstrTable [ iInstrIndex ].iOpCount = iOpCount;

	g_InstrTable [ iInstrIndex ].OpList = ( OpTypes * )
		malloc ( iOpCount * sizeof ( OpTypes ) );

	int iReturnInstrIndex = iInstrIndex;

	++ iInstrIndex;
	
	return iRetrunInstrIndex;
}

void SetOpType ( int iInstrIndex, int iOPIndex, OpTypes iOpType )
{
	g_InstrTable [ iInstrIndex ].OpList [ iOpIndex] = iOpIndex;
}

int GetInstrByMnemonic ( char * pstrMnemonic, InstrLooup * pInstr )
{
	for ( int iCurrInstrIndex = 0; iCurrInstrIndex < MAX_INSTR_LOOKUP_COUNT;
			++ iCurrInstrIndex )
	{
		if ( strcmp ( g_InstrTable [ iCurrInstrIndex ].pstrMnemonic, 
					pstrMnemonic ) == 0 )
		{
			* pInstr = g_InstrTable [ iCurrInstrIndex ];
			return true;
		}
	}
	
	return false;
}
