//
//  main.cpp
//  assembler
//
//  Created by Taoran Xue on 1/13/17.
//  Copyright © 2017 Taoran Xue. All rights reserved.
//



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ---- Filename --------------------------------------------------------------------------

#define MAX_FILENAME_SIZE           2048        // Maximum filename length

#define SOURCE_FILE_EXT             ".XASM"     // Extension of a source code file
#define EXEC_FILE_EXT               ".XSE"      // Extension of an executable code file

// ---- Source Code -----------------------------------------------------------------------

#define MAX_SOURCE_CODE_SIZE        65536       // Maximum number of lines in source
// code
#define MAX_SOURCE_LINE_SIZE        4096        // Maximum source line length

// ---- ,XSE Header -----------------------------------------------------------------------

#define XSE_ID_STRING               "XSE0"      // Written to the file to state it's
// validity

#define VERSION_MAJOR               0           // Major version number
#define VERSION_MINOR               4           // Minor version number

// ---- Lexer -----------------------------------------------------------------------------

#define MAX_LEXEME_SIZE             256         // Maximum lexeme size

#define LEX_STATE_NO_STRING         0           // Lexemes are scanned as normal
#define LEX_STATE_IN_STRING         1           // Lexemes are scanned as strings
#define LEX_STATE_END_STRING        2           // Lexemes are scanned as normal, and the
// next state is LEXEME_STATE_NOSTRING

#define TOKEN_TYPE_INT              0           // An integer literal
#define TOKEN_TYPE_FLOAT            1           // An floating-point literal
#define TOKEN_TYPE_STRING           2           // An string literal
#define TOKEN_TYPE_QUOTE            3           // A double-quote
#define TOKEN_TYPE_IDENT            4           // An identifier
#define TOKEN_TYPE_COLON            5           // A colon
#define TOKEN_TYPE_OPEN_BRACKET     6           // An openening bracket
#define TOKEN_TYPE_CLOSE_BRACKET    7           // An closing bracket
#define TOKEN_TYPE_COMMA            8           // A comma
#define TOKEN_TYPE_OPEN_BRACE       9           // An openening curly brace
#define TOKEN_TYPE_CLOSE_BRACE      10          // An closing curly brace
#define TOKEN_TYPE_NEWLINE          11          // A newline

#define TOKEN_TYPE_INSTR			12			// An instruction

#define TOKEN_TYPE_SETSTACKSIZE     13          // The SetStackSize directive
#define TOKEN_TYPE_VAR              14          // The Var/Var [] directives
#define TOKEN_TYPE_FUNC             15          // The Func directives
#define TOKEN_TYPE_PARAM            16          // The Param directives
#define TOKEN_TYPE_REG_RETVAL       17          // The _RetVal directives

#define TOKEN_TYPE_INVALID          18          // Error code for invalid tokens
#define END_OF_TOKEN_STREAM         19          // The end of the stream has been
// reached

#define MAX_IDENT_SIZE              256        // Maximum identifier size

// ---- Instruction Lookup Table ----------------------------------------------------------

#define MAX_INSTR_LOOKUP_COUNT      256         // The maximum number of instructions
// the lookup table will hold
#define MAX_INSTR_MNEMONIC_SIZE     16          // Maximum size of an instruction
// mnemonic's string

// ---- Instruction Opcodes -----------------------------------------------------------

#define INSTR_MOV               0

#define INSTR_ADD               1
#define INSTR_SUB               2
#define INSTR_MUL               3
#define INSTR_DIV               4
#define INSTR_MOD               5
#define INSTR_EXP               6
#define INSTR_NEG               7
#define INSTR_INC               8
#define INSTR_DEC               9

#define INSTR_AND               10
#define INSTR_OR                11
#define INSTR_XOR               12
#define INSTR_NOT               13
#define INSTR_SHL               14
#define INSTR_SHR               15

#define INSTR_CONCAT            16
#define INSTR_GETCHAR           17
#define INSTR_SETCHAR           18

#define INSTR_JMP               19
#define INSTR_JE                20
#define INSTR_JNE               21
#define INSTR_JG                22
#define INSTR_JL                23
#define INSTR_JGE               24
#define INSTR_JLE               25

#define INSTR_PUSH              26
#define INSTR_POP               27

#define INSTR_CALL              28
#define INSTR_RET               29
#define INSTR_CALLHOST          30

#define INSTR_PAUSE             31
#define INSTR_EXIT              32

// ---- Operand Type Bitfield Flags ---------------------------------------------------

// The following constants are used as flags into an operand type bit field, hence
// their values being increasing powers of 2.

#define OP_FLAG_TYPE_INT        1           // Integer literal value
#define OP_FLAG_TYPE_FLOAT      2           // Floating-point literal value
#define OP_FLAG_TYPE_STRING     4           // Integer literal value
#define OP_FLAG_TYPE_MEM_REF    8           // Memory reference (variable or array
// index, both absolute and relative)
#define OP_FLAG_TYPE_LINE_LABEL 16          // Line label (used for jumps)
#define OP_FLAG_TYPE_FUNC_NAME  32          // Function table index (used for Call)
#define OP_FLAG_TYPE_HOST_API_CALL  64      // Host API Call table index (used for
// CallHost)
#define OP_FLAG_TYPE_REG        128         // Register

// ---- Assembled Instruction Stream ------------------------------------------------------

#define OP_TYPE_INT                 0           // Integer literal value
#define OP_TYPE_FLOAT               1           // Floating-point literal value
#define OP_TYPE_STRING_INDEX        2           // String literal value
#define OP_TYPE_ABS_STACK_INDEX     3           // Absolute array index
#define OP_TYPE_REL_STACK_INDEX     4           // Relative array index
#define OP_TYPE_INSTR_INDEX         5           // Instruction index
#define OP_TYPE_FUNC_INDEX          6           // Function index
#define OP_TYPE_HOST_API_CALL_INDEX 7           // Host API call index
#define OP_TYPE_REG                 8           // Register

// ---- Functions -------------------------------------------------------------------------

#define MAIN_FUNC_NAME				"_MAIN"		// _Main ()'s name

// ---- Error Strings ---------------------------------------------------------------------

// The following macros are used to represent assembly-time error strings

#define ERROR_MSSG_INVALID_INPUT	\
"Invalid input"

#define ERROR_MSSG_LOCAL_SETSTACKSIZE	\
"SetStackSize can only appear in the global scope"

#define ERROR_MSSG_INVALID_STACK_SIZE	\
"Invalid stack size"

#define ERROR_MSSG_MULTIPLE_SETSTACKSIZES	\
"Multiple instances of SetStackSize illegal"

#define ERROR_MSSG_IDENT_EXPECTED	\
"Identifier expected"

#define ERROR_MSSG_INVALID_ARRAY_SIZE	\
"Invalid array size"

#define ERROR_MSSG_IDENT_REDEFINITION	\
"Identifier redefinition"

#define ERROR_MSSG_UNDEFINED_IDENT	\
"Undefined identifier"

#define ERROR_MSSG_NESTED_FUNC	\
"Nested functions illegal"

#define ERROR_MSSG_FUNC_REDEFINITION	\
"Function redefinition"

#define ERROR_MSSG_UNDEFINED_FUNC	\
"Undefined function"

#define ERROR_MSSG_GLOBAL_PARAM	\
"Parameters can only appear inside functions"

#define ERROR_MSSG_MAIN_PARAM	\
"_Main () function cannot accept parameters"

#define ERROR_MSSG_GLOBAL_LINE_LABEL	\
"Line labels can only appear inside functions"

#define ERROR_MSSG_LINE_LABEL_REDEFINITION	\
"Line label redefinition"

#define ERROR_MSSG_UNDEFINED_LINE_LABEL	\
"Undefined line label"

#define ERROR_MSSG_GLOBAL_INSTR	\
"Instructions can only appear inside functions"

#define ERROR_MSSG_INVALID_INSTR	\
"Invalid instruction"

#define ERROR_MSSG_INVALID_OP	\
"Invalid operand"

#define ERROR_MSSG_INVALID_STRING	\
"Invalid string"

#define ERROR_MSSG_INVALID_ARRAY_NOT_INDEXED	\
"Arrays must be indexed"

#define ERROR_MSSG_INVALID_ARRAY	\
"Invalid array"

#define ERROR_MSSG_INVALID_ARRAY_INDEX	\
"Invalid array index"



// ---- Data Structures ----

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

typedef struct _Instr
{
    int iOpcode;
    int iOpCount;
    Op * pOpList;
}
Instr;


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
    int iLocalDataSize;
}
FuncNode;

typedef struct _LabelNode
{
    int iIndex;
    char pstrIdent [ MAX_IDENT_SIZE ];
    int iTargetIndex;
    int iFuncIndex;
}
LabelNode;

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
    int iIndex;
    char pstrIdent [ MAX_IDENT_SIZE ];
    int iSize;
    int iStackIndex;
    int iFuncIndex;
}
SymbolNode;

typedef int Token;

typedef struct _Lexer
{
    int iCurrSourceLine;
    
    unsigned int iIndex0,
    iIndex1;
    
    Token CurrToken;
    char pstrCurrLexeme [ MAX_LEXEME_SIZE ];
    
    int iCurrLexState;
}
Lexer;

// ---- Global Variables ----
Lexer g_Lexer;

// Source code representation
char ** g_ppstrSourceCode = NULL;
int g_iSourceCodeSize;

// The instruction lookup table
InstrLookup g_InstrTable [ MAX_INSTR_LOOKUP_COUNT ];

// The assembled instruction stream
Instr * g_pInstrStream = NULL;
int g_iInstrStreamSize;

// The script header
ScriptHeader g_ScriptHeader;
int g_iIsSetStackSizeFound;

// The main tables
LinkedList g_StringTable;
LinkedList g_FuncTable;
LinkedList g_SymbolTable;
LinkedList g_LabelTable;
LinkedList g_HostAPICallTable;

// ---- Function declarations ----
int AddString ( LinkedList * pList, char * pstrString );
int AddFunc ( char * pstrName, int iEntryPoint );
FuncNode * GetFuncByName( char * pstrName );
void SetFuncInfo ( char * pstrName, int iParamCount, int iLocalDataSize );

int AddSymbol ( char * pstrIdent, int iSize, int iStackIndex, int iFuncIndex );
SymbolNode * GetSymbolByIdent ( char * pstrIdent, int iFuncIndex );
int GetStackIndexByIdent ( char * pstrIdent, int iFuncIndex );
int GetSizeByIdent ( char * pstrIdent, int iFuncIndex );

int AddLabel ( char * pstrIdent, int iTargetIndex, int iFuncIndex );
LabelNode * GetLabelByIdent ( char * pstrIdent, int iFuncIndex );

int AddInstrLookup ( char * pstrMnemonic, int iOpcode, int iOpCount );
void SetOpType ( int iInstrIndex, int iOpIndex, OpTypes iOpType );
int GetInstrByMnemonic ( char * pstrMnemonic, InstrLookup * pInstr );

int SkipToNextLine();
void ResetLexer();

void strupr(char * pstrString);
void Exit ();
void ExitOnError (char * pstrErrorMssg);
void ExitOnCodeError (char * pstrErrorMssg);
void ExitOnCharExpectedError (char cChar);

// ---- Functions ----
void AssmblSourceFile ()
{
    g_ScriptHeader.iStackSize = 0;
    g_ScriptHeader.iIsMainFuncPresent = false;
    
    g_iInstrStreamSize = 0;
    g_iIsSetStackSizeFound = false;
    g_ScriptHeader.iGlobalDataSize = 0;
    
    int iIsFuncActive = false;
    FuncNode * pCurrFunc;
    int iCurrFuncIndex;
    char pstrCurrFuncName[MAX_IDENT_SIZE];
    int iCurrFuncParamCount = 0;
    int iCurrFuncLocalDataSize = 0;
    
    InstrLookup CurrInstr;
    
    ResetLexer();
    
    while (true)
    {
        if (GetNextToken () == END_OF_TOKEN_STREAM)
            break;
        
        switch (g_Lexer.CurrToken)
        {
            case TOKEN_TYPE_SETSTACKSIZE:
            {
                if (iIsFuncActive)
                    ExitOnCodeError (ERROR_MSSG_LOCAL_SETSTACKSIZE);
                
                if (g_iIsSetStackSizeFound)
                    ExitOnCodeError (ERROR_MSSG_MULTIPLE_SETSTACKSIZES);
                
                if (GetNextToken () != TOKEN_TYPE_INT)
                    ExitOnCodeError (ERROR_MSSG_INVALID_STACK_SIZE);
                
                g_ScriptHeader.iStackSize = atoi (GetCurrLexeme ());
                
                g_iIsSetStackSizeFound = true;
                
                break;
            }
                
            case TOKEN_TYPE_FUNC:
            {
                if (iIsFuncActive)
                    ExitOnCodeError (ERROR_MSSG_NESTED_FUNC);
                
                if (GetNextToken () != TOKEN_TYPE_IDENT)
                    ExitOnCodeError (ERROR_MSSG_IDENT_EXPECTED);
                char * pstrFuncName = GetCurrLexeme ();
                
                int iEntryPoint = g_iInstrStreamSize;
                
                int iFuncIndex = AddFunc (pstrFuncName, iEntryPoint);
                if (iFuncIndex == -1)
                    ExitOnCodeError (ERROR_MSSG_FUNC_REDEFINITION);
                
                if (strcmp (pstrFuncName, MAIN_FUNC_NAME) == 0)
                {
                    g_ScriptHeader.iIsMainFuncPresent = true;
                    g_ScriptHeader.iMainFuncIndex = iFuncIndex;
                }
                
                iIsFuncActive = true;
                strcpy (pstrCurrFuncName, pstrFuncName);
                iCurrFuncIndex = iFuncIndex;
                iCurrFuncParamCount = 0;
                iCurrFuncLocalDataSize = 0;
                
                while (GetNextToken () == TOKEN_TYPE_OPEN_BRACE);
                
                if (g_Lexer.CurrToken != TOKEN_TYPE_OPEN_BRACE)
                    ExitOnCharExpectedError ('{');
                
                ++ g_iInstrStreamSize;
                
                break;
            }
                
            case TOKEN_TYPE_CLOSE_BRACE:
            {
                if (!iIsFuncActive)
                    ExitOnCharExpectedError ('}');
                
                SetFuncInfo (pstrCurrFuncName, iCurrFuncParamCount,
                             iCurrFuncLocalDataSize);
                
                iIsFuncActive = false;
                break;
            }
                
            case TOKEN_TYPE_VAR:
            {
                if (GetNextToken () != TOKEN_TYPE_IDENT)
                    ExitOnCodeError (ERROR_MSSG_IDENT_EXPECTED);
                char pstrIdent[MAX_IDENT_SIZE];
                strcpy (pstrIdent, GetCurrLexeme ());
                
                int iSize = 1;
                
                if (GetLookAheadChar () == '[')
                {
                    if (GetNextToken () != TOKEN_TYPE_OPEN_BRACKET)
                        ExitOnCharExpectedError('[');
                    
                    if (GetNextToken () != TOKEN_TYPE_INT)
                        ExitOnCodeError (ERROR_MSSG_INVALID_ARRAY_SIZE);
                    iSize = atoi (GetCurrLexeme ());
                    
                    if (iSize <= 0)
                        ExitOnCodeError (ERROR_MSSG_INVALID_ARRAY_SIZE);
                    if (GetNextToken () != TOKEN_TYPE_CLOSE_BRACKET)
                        ExitOnCharExpectedError (']');
                }
                
                int iStackIndex;
                if (iIsFuncActive)
                {
                    iStackIndex = -(iCurrFuncLocalDataSize + 2);
                }
                else
                {
                    iStackIndex = g_ScriptHeader.iGlobalDataSize;
                }
                
                if (AddSymbol (pstrIdent, iSize, iStackIndex, iCurrFuncIndex) == -1)
                    ExitOnCodeError (ERROR_MSSG_IDENT_REDEFINITION);
                
                if (iIsFuncActive)
                {
                    iCurrFuncLocalDataSize += iSize;
                }
                else
                {
                    g_ScriptHeader.iGlobalDataSize += iSize;
                }
                
                break;
            }
                
        }
        
        void strupr (char * pstrString)
        {
            for (int iCurrCharIndex = 0;
                 iCurrCharIndex < strlen (pstrString); ++ iCurrCharIndex)
            {
                char cCurrChar = pstrString[iCurrCharIndex];
                pstrString[iCurrCharIndex] = toupper(cCurrChar);
            }
        }
        
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
        
        int AddString( LinkedList * pList, char * pstrString )
        {
            LinkedListNode * pNode = pList->pHead;
            
            for ( int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++ iCurrNode )
            {
                if ( strcmp ( ( char * ) pNode->pData, pstrString ) == 0 )
                    return iCurrNode;
                
                pNode = pNode->pNext;
            }
            
            char * pstrStringNode = (char *) malloc (strlen ( pstrString ) + 1);
            strcpy ( pstrStringNode, pstrString );
            
            return AddNode ( pList, pstrStringNode );
        }
        
        int AddFunc (char * pstrName, int iEntryPoint)
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
            FuncNode * pFunc = GetFuncByName ( pstrName );
            
            pFunc->iParamCount = iParamCount;
            pFunc->iLocalDataSize = iLocalDataSize;
        }
        
        FuncNode * GetFuncByName ( char * pstrName )
        {
            if ( ! g_FuncTable.iNodeCount )
                return NULL;
            
            LinkedListNode * pCurrNode = g_FuncTable.pHead;
            
            for (int iCurrNode = 0; iCurrNode < g_FuncTable.iNodeCount; ++ iCurrNode)
            {
                FuncNode * pCurrFunc = ( FuncNode * ) pCurrNode->pData;
                
                if ( strcmp ( pCurrFunc->pstrName, pstrName ) == 0 )
                    return pCurrFunc;
                
                pCurrNode = pCurrNode->pNext;
            }
            return NULL;
        }
        
        int AddSymbol ( char * pstrIdent, int iSize, int iStackIndex, int iFuncIndex )
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
        
        SymbolNode * GetSymbolByIdent ( char * pstrIdent, int iFuncIndex )
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
                        return pCurrSymbol;
                
                pCurrNode = pCurrNode->pNext;
            }
            
            return NULL;
        }
        
        int GetStackIndexByIndent ( char * pstrIdent, int iFuncIndex )
        {
            SymbolNode * pSymbol = GetSymbolByIdent ( pstrIdent, iFuncIndex );
            
            return pSymbol->iStackIndex;
        }
        
        int GetSizeByIndent ( char * pstrIdent, int iFuncIndex )
        {
            SymbolNode * pSymbol = GetSymbolByIdent ( pstrIdent, iFuncIndex );
            
            return pSymbol->iSize;
        }
        
        int AddLabel( char * pstrIdent, int iTargetIndex, int iFuncIndex)
        {
            if ( GetLabelByIdent ( pstrIdent, iFuncIndex) )
                return -1;
            
            LabelNode * pNewLabel = (LabelNode *) malloc ( sizeof ( LabelNode) );
            
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
                
                pCurrNode = pCurrNode->pNext;
            }
            
            return NULL;
        }
        
        int AddInstrLookup ( char * pstrMnemonic, int iOpcode, int iOpCount )
        {
            static int iInstrIndex = 0;
            
            if ( iInstrIndex >= MAX_INSTR_LOOKUP_COUNT )
                return -1;
            
            strcpy ( g_InstrTable [ iInstrIndex ].pstrMnemonic, pstrMnemonic );
            strupr ( g_InstrTable [ iInstrIndex ].pstrMnemonic );
            g_InstrTable [ iInstrIndex ].iOpcode = iOpcode;
            g_InstrTable [ iInstrIndex ].iOpCount = iOpCount;
            
            g_InstrTable [ iInstrIndex ].OpList = ( OpTypes * )
            malloc ( iOpCount * sizeof ( OpTypes ) );
            
            int iReturnInstrIndex = iInstrIndex;
            
            ++ iInstrIndex;
            
            return iReturnInstrIndex;
        }
        
        void SetOpType ( int iInstrIndex, int iOpIndex, OpTypes iOpType )
        {
            g_InstrTable [ iInstrIndex ].OpList [ iOpIndex] = iOpIndex;
        }
        
        int GetInstrByMnemonic ( char * pstrMnemonic, InstrLookup * pInstr )
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
        
        int SkipToNextLine ()
        {
            ++ g_Lexer.iCurrSourceLine;
            
            if (g_Lexer.iCurrSourceLine >= g_iSourceCodeSize)
                return false;
            
            g_Lexer.iIndex0 = 0;
            g_Lexer.iIndex1 = 0;
            
            g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;
            
            return true;
        }
        
        void ResetLexer ();
        {
            g_Lexer.iCurrSourceLine = 0;
            
            g_Lexer.iIndex0 = 0;
            g_Lexer.iIndex1 = 0;
            
            g_Lexer.CurrToken = TOKEN_TYPE_INVALID;
            
            g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;
        }
        
        char GetLookAheadChar ()
        {
            int iCurrSourceLine = g_Lexer.iCurrSourceLine;
            unsigned int iIndex = g_Lexer.iIndex1;
            
            if (g_Lexer.iCurrLexState != LEX_STATE_IN_STRING)
            {
                while (true)
                {
                    if (iIndex >= strlen (g_ppstrSourceCode[iCurrSourceLine]))
                    {
                        iCurrSourceLine += 1;
                        
                        if (iCurrSourceLine >= g_iSourceCodeSize)
                            return 0;
                        
                        iIndex = 0;
                    }
                    
                    if (!IsCharWhitespace(g_ppstrSourceCode[iCurrSourceLine][iIndex]))
                        break;
                    
                    ++ index;
                }
                
                return g_ppstrSourceCode[iCurrSourceLine][iIndex];
            }
            
            
            void StripComments (char * pstrSourceLine)
            {
                unsigned int iCurrCharIndex;
                int iInString;
                
                iInString = 0;
                for (iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrSourceLine) - 1; ++ iCurrCharIndex)
                {
                    if (pstrSourceLine[iCurrCharIndex] == '"')
                    {
                        if (iInString)
                            iInString = 0;
                        else
                            iInString = 1;
                    }
                    
                    if (pstrSourceLine[iCurrCharIndex] == ';')
                    {
                        if (!iInString)
                        {
                            pstrSourceLine[iCurrCharIndex] = '\n';
                            pstrSourceLine[iCurrCharIndex + 1] = '\0';
                            break;
                        }
                    }
                }
            }
            
            int IsCharWhitespace (char cChar)
            {
                if (cChar == ' ' || cChar == '\t')
                    return true;
                else
                    return false;
            }
            
            int IsCharNumeric (char cChar)
            {
                if (cChar >= '0' && cChar <= '9')
                    return true;
                else
                    return false;
            }
            
            int IsCharIdent (char cChar)
            {
                if ((cChar >= '0' && cChar <= '9') ||
                    (cChar >= 'A' && cChar <= 'Z') ||
                    (cChar >= 'a' && cChar <= 'z') ||
                    cChar == '_')
                    return true;
                else
                    return false;
            }
            
            int IsCharDelimiter (char cChar)
            {
                if (cChar == ':' || cChar == ',' || cChar == '"' ||
                    cChar == '[' || cChar == ']' || cChar == '{' ||
                    cChar == '}' || IsCharWhitespace (cChar) || cChar == '\n')
                    return true;
                else
                    return false;
            }
            
            void TrimWhitespace (char * pstrString)
            {
                unsigned int iStringLength = strlen (pstrString);
                unsigned int iPadLength;
                unsigned int iCurrCharIndex;
                
                if (iStringLength > 1)
                {
                    for (iCurrCharIndex = 0; iCurrCharIndex < iStringLength; ++ iCurrCharIndex)
                        if (!IsCharWhitespace (pstrString[iCurrCharIndex]))
                            break;
                    
                    iPadLength = iCurrCharIndex;
                    if (iPadLength)
                    {
                        for (iCurrCharIndex = iPadLength; iCurrCharIndex < iStringLength;
                             ++ iCurrCharIndex)
                            pstrString[iCurrCharIndex - iPadLength] = pstrString[iCurrCharIndex];
                        
                        for (iCurrCharIndex = iStringLength - iPadLength;
                             iCurrCharIndex < iStringLength; ++ iCurrCharIndex)
                            pstrString[iCurrCharIndex] = ' ';
                    }
                    
                    for (iCurrCharIndex = iStringLength - 1; iCurrCharIndex > 0;
                         -- iCurrCharIndex)
                    {
                        if (!IsCharWhitespace(pstrString[iCurrCharIndex]))
                        {
                            pstrString[iCurrCharIndex + 1] = '\0';
                            break;
                        }
                    }
                }
            }
            
            int IsStringWhitespace (char * pstrString)
            {
                if (!pstrString)
                    return false;
                
                if (strlen (pstrString) == 0)
                    return true;
                
                for (unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString);
                     ++ iCurrCharIndex)
                    if (!IsCharWhitespace (pstrString[iCurrCharIndex]) && 
                        pstrString[iCurrCharIndex != '\n'])
                        return false;
                return true;
            }
            
            int IsStringIdent (char * pstrString)
            {
                if (!pstrString)
                    return false;
                
                if (strlen(pstrString) == 0)
                    return false;
                
                if (pstrString[0] >= '0' && pstrString[0] <= '9')
                    return false;
                
                for (unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString);
                     ++ iCurrCharIndex)
                    if (!IsCharIdent(pstrString[iCurrCharIndex]))
                        return false;
                return true;
            }
            
            int IsStringInteger (char * pstrString)
            {
                if (!pstrString)
                    return false;
                
                if (strlen(pstrString) == 0)
                    return false;
                
                unsigned int iCurrCharIndex;
                
                for (iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString); ++ iCurrCharIndex)
                    if (!IsCharNumeric (pstrString[iCurrCharIndex]) && 
                        !(pstrString[iCurrCharIndex] == '-'))
                        return false;
                
                for (iCurrCharIndex = 1; iCurrCharIndex < strlen (pstrString); ++ iCurrCharIndex)
                    if (pstrString[iCurrCharIndex == '-'])
                        return false;
                
                return true;
            }
            
            
            int IsStringFloat(char * pstrString)
            {
                if (!pstrString)
                    return false;
                
                if (strlen (pstrString) == 0)
                    return false;
                
                unsigned int iCurrCharIndex;
                
                for (iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString); ++ iCurrCharIndex)
                    if (!IsCharNumeric (pstrString[iCurrCharIndex]) && 
                        !(pstrString[iCurrCharIndex] == '.') &&
                        !(pstrString[iCurrCharIndex] == '-'))
                        return false;
                
                int iRadixPointFound = 0;
                
                for (iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString); ++ iCurrCharIndex)
                {
                    if (pstrString[iCurrCharIndex] == '.')
                    {
                        if (iRadixPointFound)
                            return false;
                        else
                            iRadixPointFound = 1;
                    }
                }
                
                for (iCurrCharIndex = 1; iCurrCharIndex < strlen (pstrString); ++ iCurrCharIndex)
                    if (pstrString[iCurrCharIndex] == '-')
                        return false;
                
                if (iRadixPointFound)
                    return true;
                else
                    return false;
            }
            
            
            
            
            
            
            Token GetNextToken ()
            {
                g_Lexer.iIndex0 = g_Lexer.iIndex1;
                
                if ( g_Lexer.iIndex0 >= strlen ( g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ]))
                {
                    if (!SkipToNextLine())
                        return END_OF_TOKEN_STREAM;
                }
                
                if (g_Lexer.iCurrLexState == LEX_STATE_END_STRING)
                    g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;
                
                if (g_Lexer.iCurrLexState != LEX_STATE_IN_STRING)
                {
                    while (true)
                    {
                        if (!IsCharWhitespace(g_ppstrSourceCode[g_Lexer.iCurrSourceLine]
                                              [g_Lexer.iIndex0]))
                            break;
                        
                        ++ g_Lexer.iIndex0;
                    }
                }
                
                g_Lexer.iIndex1 = g_Lexer.iIndex0;
                
                while (true)
                {
                    if (g_Lexer.iCurrLexState == LEX_STATE_IN_STRING)
                    {
                        if (g_Lexer.iIndex1 >= 
                            strlen (g_ppstrSourceCode[g_Lexer.iCurrSourceLine]))
                        {
                            g_Lexer.CurrToken = TOKEN_TYPE_INVALID;
                            return g_Lexer.CurrToken;
                        }
                        
                        if (g_ppstrSourceCode[g_Lexer.iCurrSourceLine][g_Lexer.iIndex1] == '\\')
                        {
                            g_Lexer.iIndex1 += 2;
                            continue;
                        }
                        
                        if (g_ppstrSourceCode[g_Lexer.iCurrSourceLine][g_Lexer.iIndex1] == '"')
                            break;
                        
                        ++ g_Lexer.iIndex1;
                    }
                    
                    else
                    {
                        if (g_Lexer.iIndex1 >= 
                            strlen (g_ppstrSourceCode[g_Lexer.iCurrSourceLine]))
                            break;
                        
                        if (IsCharDelimiter (g_ppstrSourceCode[g_Lexer.iCurrSourceLine]
                                             [g_Lexer.iIndex1]))
                            break;	
                        
                        ++ g_Lexer.iIndex1;
                    }
                }
                
                if (g_Lexer.iIndex1 - g_Lexer.iIndex0 == 0)
                    ++ g_Lexer.iIndex1;
                
                unsigned int iCurrDestIndex = 0;
                for (unsigned int iCurrSourceIndex = g_Lexer.iIndex0; 
                     iCurrSourceIndex < g_Lexer.iIndex1; ++ iCurrSourceIndex)
                {
                    if (g_Lexer.iCurrLexState == LEX_STATE_IN_STRING)
                        if (g_ppstrSourceCode[g_Lexer.iCurrSourceLine][iCurrSourceIndex] 
                            == '\\')
                            ++ iCurrSourceIndex;
                    
                    g_Lexer.pstrCurrLexeme[iCurrDestIndex] = 
                    g_ppstrSourceCode[g_Lexer.iCurrSourceLine][iCurrSourceIndex];
                    
                    ++ iCurrDestIndex;
                }
                
                g_Lexer.pstrCurrLexeme[iCurrDestIndex] = '\0';
                
                if (g_Lexer.iCurrLexState != LEX_STATE_IN_STRING)
                    strupr (g_Lexer.pstrCurrLexeme);
                
                g_Lexer.CurrToken = TOKEN_TYPE_INVALID;
                
                if (strlen(g_Lexer.pstrCurrLexeme) > 1 || g_Lexer.pstrCurrLexeme[0] != '"')
                {
                    if (g_Lexer.iCurrLexState == LEX_STATE_IN_STRING)
                    {
                        g_Lexer.CurrToken = TOKEN_TYPE_STRING;
                        return TOKEN_TYPE_STRING;
                    }
                }
                
                if (strlen (g_Lexer.pstrCurrLexeme) == 1)
                {
                    switch (g_Lexer.pstrCurrLexeme[0])
                    {
                        case '"':
                            switch (g_Lexer.iCurrLexState)
                        {
                            case LEX_STATE_NO_STRING:
                                g_Lexer.iCurrLexState = LEX_STATE_IN_STRING;
                                break;
                                
                            case LEX_STATE_IN_STRING:
                                g_Lexer.iCurrLexState = LEX_STATE_END_STRING;
                                break;
                        }
                            
                            g_Lexer.CurrToken = TOKEN_TYPE_QUOTE;
                            
                        case ',':
                            g_Lexer.CurrToken = TOKEN_TYPE_COMMA;
                            break;
                            
                        case ':':
                            g_Lexer.CurrToken = TOKEN_TYPE_COLON;
                            break;
                            
                        case '[':
                            g_Lexer.CurrToken = TOKEN_TYPE_OPEN_BRACKET;
                            break;
                            
                        case ']':
                            g_Lexer.CurrToken = TOKEN_TYPE_CLOSE_BRACKET;
                            break;
                            
                        case '{':
                            g_Lexer.CurrToken = TOKEN_TYPE_OPEN_BRACE;
                            break;
                            
                        case '}':
                            g_Lexer.CurrToken = TOKEN_TYPE_CLOSE_BRACE;
                            break;
                            
                        case '\n':
                            g_Lexer.CurrToken = TOKEN_TYPE_NEWLINE;
                            break;
                    }
                }
                
                if (IsStringInteger (g_Lexer.pstrCurrLexeme))
                    g_Lexer.CurrToken = TOKEN_TYPE_INT;
                
                if (IsStringFloat (g_Lexer.pstrCurrLexeme))
                    g_Lexer.CurrToken = TOKEN_TYPE_FLOAT;
                
                if (IsStringIdent (g_Lexer.pstrCurrLexeme))
                    g_Lexer.CurrToken = TOKEN_TYPE_IDENT;
                
                if (strcmp(g_Lexer.pstrCurrLexeme, "SETSTACKSIZE") == 0)
                    g_Lexer.CurrToken = TOKEN_TYPE_SETSTACKSIZE;
                
                if (strcmp(g_Lexer.pstrCurrLexeme, "VAR") == 0)
                    g_Lexer.CurrToken = TOKEN_TYPE_VAR;
                
                if (strcmp(g_Lexer.pstrCurrLexeme, "FUNC") == 0)
                    g_Lexer.CurrToken = TOKEN_TYPE_FUNC;
                
                if (strcmp(g_Lexer.pstrCurrLexeme, "_RETVAL") == 0)
                    g_Lexer.CurrToken = TOKEN_TYPE_REG_RETVAL;
                
                InstrLookup Instr;
                
                if (GetInstrByMnemonic(g_Lexer.pstrCurrLexeme, & Instr))
                    g_Lexer.CurrToken = TOKEN_TYPE_INSTR;
                
                return g_Lexer.CurrToken;
            }
            
            void ExitOnError (char * pstrErrorMssg)
            {
                printf ("Fatal Error: %s.\n", pstrErrorMssg);
                
                Exit ();
            }
            
            void ExitOnCodeError (char * pstrErrorMssg)
            {
                printf ("Error: %s. \n\n", pstrErrorMssg);
                printf ("Line %d\n", g_Lexer.iCurrSourceLine);
                
                char pstrSourceLine[MAX_SOURCE_LINE_SIZE];
                strcpy (pstrSourceLine, g_ppstrSourceCode[g_Lexer.iCurrSourceLine]);
                
                for (unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen(pstrSourceLine); 
                     ++ icurrchar)
                {
                    if (pstrSourceLine[iCurrCharIndex] == '\t')
                        pstrSourceLine[iCurrFuncIndex] = ' ';
                }
                
                printf ("%s", pstrSourceLine);
                
                for (unsigned int iCurrSpace = 0; iCurrSpace < g_Lexer.iIndex0; ++ iCurrSpace)
                    printf (" ");
                printf ("^\n");
                
                printf ("Could not assemble %s.\n", g_pstrSourceFilename);
                
                Exit();
            }
            
            void ExitOnCharExpectedError (char cChar)
            {
                char * pstrErrorMssg = (char *) malloc (strlen ("' ' expected"));
                sprintf (pstrErrorMssg, "'%c' expected", cChar);
                
                ExitOnCodeError(pstrErrorMssg);
            }
