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
#include "main.h"


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
            
            case TOKEN_TYPE_IDENT:
            {
                if (GetLookAheadChar() != ':')
                    ExitOnCodeError(ERROR_MSSG_INVALID_INSTR);
                
                if (!iIsFuncActive)
                    ExitOnCodeError(ERROR_MSSG_GLOBAL_LINE_LABEL);
                
                char * pstrIdent = GetCurrLexeme();
                
                int iTargetIndex = g_iInstrStreamSize - 1;
                
                int iFuncIndex = iCurrFuncIndex;
                
                if (AddLabel(pstrIdent, iTargetIndex, iFuncIndex) == -1)
                    ExitOnCodeError(ERROR_MSSG_LINE_LABEL_REDEFINITION);
                
                break;
                
            }
            
        }
        
        while (true)
        {
            if (GetNextToken() == END_OF_TOKEN_STREAM)
                break;
            
            switch (g_Lexer.CurrToken) {
                case TOKEN_TYPE_PARAM:
                {
                    if (GetNextToken() != TOKEN_TYPE_IDENT)
                        ExitOnCodeError(ERROR_MSSG_IDENT_EXPECTED);
                    
                    char * pstrIdent = GetCurrLexeme();
                    
                    int iStackIndex = - (pCurrFunc->iLocalDataSize + 2 + (iCurrFuncParamCount + 1));
                    
                    if (AddSymbol(pstrIdent, 1, iStackIndex, iCurrFuncIndex) == -1)
                        ExitOnCodeError(ERROR_MSSG_IDENT_REDEFINITION);
                    
                    ++ iCurrFuncParamCount;
                    
                    break;
                }
                    
                case TOKEN_TYPE_INSTR:
                {
                    GetInstrByMnemonic(GetCurrLexeme(), & CurrInstr);
                    g_pInstrStream[g_iCurrInstrIndex].iOpcode = CurrInstr.iOpcode;
                    g_pInstrStream[g_iCurrInstrIndex].iOpCount = CurrInstr.iOpCount;
                    Op * pOpList = (Op *) malloc(CurrInstr.iOpCount * sizeof(Op));
                    
                    for (int iCurrOpIndex = 0; iCurrOpIndex < CurrInstr.iOpCount; ++ iCurrOpIndex)
                    {
                        OpTypes CurrOpTypes = CurrInstr.OpList[iCurrOpIndex];
                        Token InitOpToken = GetNextToken();
                        
                        switch (InitOpToken) {
                                
                            case TOKEN_TYPE_INT:
                            {
                                if (CurrOpTypes & OP_FLAG_TYPE_INT)
                                {
                                    pOpList[iCurrOpIndex].iType = OP_TYPE_INT;
                                    pOpList[iCurrOpIndex].iIntLiteral = atoi(GetCurrLexeme());
                                }
                                else
                                    ExitOnCodeError(ERROR_MSSG_INVALID_OP);
                                break;
                            }
                                
                                // A floating-point literal
                                
                            case TOKEN_TYPE_FLOAT:
                            {
                                // Make sure the operand type is valid
                                
                                if ( CurrOpTypes & OP_FLAG_TYPE_FLOAT )
                                {
                                    // Set a floating-point operand type
                                    
                                    pOpList [ iCurrOpIndex ].iType = OP_TYPE_FLOAT;
                                    
                                    // Copy the value into the operand list from the current
                                    // lexeme
                                    
                                    pOpList [ iCurrOpIndex ].fFloatLiteral = ( float ) atof ( GetCurrLexeme () );
                                }
                                else
                                    ExitOnCodeError ( ERROR_MSSG_INVALID_OP );
                                
                                break;
                                
                                // A string literal (since strings always start with quotes)
                            }
                                
                            case TOKEN_TYPE_QUOTE:
                            {
                                // Make sure the operand type is valid
                                
                                if ( CurrOpTypes & OP_FLAG_TYPE_STRING )
                                {
                                    GetNextToken ();
                                    
                                    // Handle the string based on its type
                                    
                                    switch ( g_Lexer.CurrToken )
                                    {
                                            // If we read another quote, the string is empty
                                            
                                        case TOKEN_TYPE_QUOTE:
                                        {
                                            // Convert empty strings to the integer value zero
                                            
                                            pOpList [ iCurrOpIndex ].iType = OP_TYPE_INT;
                                            pOpList [ iCurrOpIndex ].iIntLiteral = 0;
                                            break;
                                        }
                                            
                                            // It's a normal string
                                            
                                        case TOKEN_TYPE_STRING:
                                        {
                                            // Get the string literal
                                            
                                            char * pstrString = GetCurrLexeme ();
                                            
                                            // Add the string to the table, or get the index of
                                            // the existing copy
                                            
                                            int iStringIndex = AddString ( & g_StringTable, pstrString );
                                            
                                            // Make sure the closing double-quote is present
                                            
                                            if ( GetNextToken () != TOKEN_TYPE_QUOTE )
                                                ExitOnCharExpectedError ( '\\' );
                                            
                                            // Set the operand type to string index and set its
                                            // data field
                                            
                                            pOpList [ iCurrOpIndex ].iType = OP_TYPE_STRING_INDEX;
                                            pOpList [ iCurrOpIndex ].iStringTableIndex = iStringIndex;
                                            break;
                                        }
                                            
                                            // The string is invalid
                                            
                                        default:
                                            ExitOnCodeError ( ERROR_MSSG_INVALID_STRING );
                                    }
                                }
                                else
                                    ExitOnCodeError ( ERROR_MSSG_INVALID_OP );
                                
                                break;
                            }
                                
                                // _RetVal
                                
                            case TOKEN_TYPE_REG_RETVAL:
                            {
                                // Make sure the operand type is valid
                                
                                if ( CurrOpTypes & OP_FLAG_TYPE_REG )
                                {
                                    // Set a register type
                                    
                                    pOpList [ iCurrOpIndex ].iType = OP_TYPE_REG;
                                    pOpList [ iCurrOpIndex ].iReg = 0;
                                }
                                else
                                    ExitOnCodeError ( ERROR_MSSG_INVALID_OP );
                                
                                break;
                                
                                // Identifiers
                                
                                // These operands can be any of the following
                                //      - Variables/Array Indices
                                //      - Line Labels
                                //      - Function Names
                                //      - Host API Calls
                            }
                                
                            case TOKEN_TYPE_IDENT:
                            {
                                // Find out which type of identifier is expected. Since no
                                // instruction in XVM assebly accepts more than one type
                                // of identifier per operand, we can use the operand types
                                // alone to determine which type of identifier we're
                                // parsing.
                                
                                // Parse a memory reference-- a variable or array index
                                
                                if ( CurrOpTypes & OP_FLAG_TYPE_MEM_REF )
                                {
                                    // Whether the memory reference is a variable or array
                                    // index, the current lexeme is the identifier so save a
                                    // copy of it for later
                                    
                                    char pstrIdent [ MAX_IDENT_SIZE ];
                                    strcpy ( pstrIdent, GetCurrLexeme () );
                                    
                                    // Make sure the variable/array has been defined
                                    
                                    if ( ! GetSymbolByIdent ( pstrIdent, iCurrFuncIndex ) )
                                        ExitOnCodeError ( ERROR_MSSG_UNDEFINED_IDENT );
                                    
                                    // Get the identifier's index as well; it may either be
                                    // an absolute index or a base index
                                    
                                    int iBaseIndex = GetStackIndexByIdent ( pstrIdent, iCurrFuncIndex );
                                    
                                    // Use the lookahead character to find out whether or not
                                    // we're parsing an array
                                    
                                    if ( GetLookAheadChar () != '[' )
                                    {
                                        // It's just a single identifier so the base index we
                                        // already saved is the variable's stack index
                                        
                                        // Make sure the variable isn't an array
                                        
                                        if ( GetSizeByIdent ( pstrIdent, iCurrFuncIndex ) > 1 )
                                            ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY_NOT_INDEXED );
                                        
                                        // Set the operand type to stack index and set the data
                                        // field
                                        
                                        pOpList [ iCurrOpIndex ].iType = OP_TYPE_ABS_STACK_INDEX;
                                        pOpList [ iCurrOpIndex ].iIntLiteral = iBaseIndex;
                                    }
                                    else
                                    {
                                        // It's an array, so lets verify that the identifier is
                                        // an actual array
                                        
                                        if ( GetSizeByIdent ( pstrIdent, iCurrFuncIndex ) == 1 )
                                            ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY );
                                        
                                        // First make sure the open brace is valid
                                        
                                        if ( GetNextToken () != TOKEN_TYPE_OPEN_BRACKET )
                                            ExitOnCharExpectedError ( '[' );
                                        
                                        // The next token is the index, be it an integer literal
                                        // or variable identifier
                                        
                                        Token IndexToken = GetNextToken ();
                                        
                                        if ( IndexToken == TOKEN_TYPE_INT )
                                        {
                                            // It's an integer, so determine its value by
                                            // converting the current lexeme to an integer
                                            
                                            int iOffsetIndex = atoi ( GetCurrLexeme () );
                                            
                                            // Add the index to the base index to find the offset
                                            // index and set the operand type to absolute stack
                                            // index
                                            
                                            pOpList [ iCurrOpIndex ].iType = OP_TYPE_ABS_STACK_INDEX;
                                            pOpList [ iCurrOpIndex ].iStackIndex = iBaseIndex + iOffsetIndex;
                                        }
                                        else if ( IndexToken == TOKEN_TYPE_IDENT )
                                        {
                                            // It's an identifier, so save the current lexeme
                                            
                                            char * pstrIndexIdent = GetCurrLexeme ();
                                            
                                            // Make sure the index is a valid array index, in
                                            // that the identifier represents a single variable
                                            // as opposed to another array
                                            
                                            if ( ! GetSymbolByIdent ( pstrIndexIdent, iCurrFuncIndex ) )
                                                ExitOnCodeError ( ERROR_MSSG_UNDEFINED_IDENT );
                                            
                                            if ( GetSizeByIdent ( pstrIndexIdent, iCurrFuncIndex ) > 1 )
                                                ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY_INDEX );
                                            
                                            // Get the variable's stack index and set the operand
                                            // type to relative stack index
                                            
                                            int iOffsetIndex = GetStackIndexByIdent ( pstrIndexIdent, iCurrFuncIndex );
                                            
                                            pOpList [ iCurrOpIndex ].iType = OP_TYPE_REL_STACK_INDEX;
                                            pOpList [ iCurrOpIndex ].iStackIndex = iBaseIndex;
                                            pOpList [ iCurrOpIndex ].iOffsetIndex = iOffsetIndex;
                                        }
                                        else
                                        {
                                            // Whatever it is, it's invalid
                                            
                                            ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY_INDEX );
                                        }
                                        
                                        // Lastly, make sure the closing brace is present as well
                                        
                                        if ( GetNextToken () != TOKEN_TYPE_CLOSE_BRACKET )
                                            ExitOnCharExpectedError ( '[' );
                                    }
                                }
                                
                                // Parse a line label
                                
                                if ( CurrOpTypes & OP_FLAG_TYPE_LINE_LABEL )
                                {
                                    // Get the current lexeme, which is the line label
                                    
                                    char * pstrLabelIdent = GetCurrLexeme ();
                                    
                                    // Use the label identifier to get the label's information
                                    
                                    LabelNode * pLabel = GetLabelByIdent ( pstrLabelIdent, iCurrFuncIndex );
                                    
                                    // Make sure the label exists
                                    
                                    if ( ! pLabel )
                                        ExitOnCodeError ( ERROR_MSSG_UNDEFINED_LINE_LABEL );
                                    
                                    // Set the operand type to instruction index and set the
                                    // data field
                                    
                                    pOpList [ iCurrOpIndex ].iType = OP_TYPE_INSTR_INDEX;
                                    pOpList [ iCurrOpIndex ].iInstrIndex = pLabel->iTargetIndex;
                                }
                                
                                // Parse a function name
                                
                                if ( CurrOpTypes & OP_FLAG_TYPE_FUNC_NAME )
                                {
                                    // Get the current lexeme, which is the function name
                                    
                                    char * pstrFuncName = GetCurrLexeme ();
                                    
                                    // Use the function name to get the function's information
                                    
                                    FuncNode * pFunc = GetFuncByName ( pstrFuncName );
                                    
                                    // Make sure the function exists
                                    
                                    if ( ! pFunc )
                                        ExitOnCodeError ( ERROR_MSSG_UNDEFINED_FUNC );
                                    
                                    // Set the operand type to function index and set its data
                                    // field
                                    
                                    pOpList [ iCurrOpIndex ].iType = OP_TYPE_FUNC_INDEX;
                                    pOpList [ iCurrOpIndex ].iFuncIndex = pFunc->iIndex;
                                }
                                
                                // Parse a host API call
                                
                                if ( CurrOpTypes & OP_FLAG_TYPE_HOST_API_CALL )
                                {
                                    // Get the current lexeme, which is the host API call
                                    
                                    char * pstrHostAPICall = GetCurrLexeme ();
                                    
                                    // Add the call to the table, or get the index of the
                                    // existing copy
                                    
                                    int iIndex = AddString ( & g_HostAPICallTable, pstrHostAPICall );
                                    
                                    // Set the operand type to host API call index and set its
                                    // data field
                                    
                                    pOpList [ iCurrOpIndex ].iType = OP_TYPE_HOST_API_CALL_INDEX;
                                    pOpList [ iCurrOpIndex ].iHostAPICallIndex = iIndex;
                                }
                                
                                break;
                            }
                                
                                // Anything else
                                
                            default:
                                
                                ExitOnCodeError ( ERROR_MSSG_INVALID_OP );
                                break;
                        }
                        
                        // Make sure a comma follows the operand, unless it's the last one
                        
                        
                        if (iCurrOpIndex < CurrInstr.iOpCount - 1)
                        {
                            if (GetNextToken() != TOKEN_TYPE_COMMA)
                                ExitOnCharExpectedError(',');
                        }
                    }
                    
                    if (GetNextToken() != TOKEN_TYPE_NEWLINE)
                        ExitOnCodeError(ERROR_MSSG_INVALID_INPUT);
                    
                    g_pInstrStream[g_iCurrInstrIndex].pOpList = pOpList;
                    
                    ++ g_iCurrInstrIndex;
                    
                }
            }
        }
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

char * GetCurrLexeme ()
{
    return g_Lexer.pstrCurrLexeme;
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

LabelNode * GetLabelByIdent (char * pstrIdent, int iFuncIndex)
{
    if (!g_LabelTable.iNodeCount)
        return NULL;
    
    LinkedListNode * pCurrNode = g_LabelTable.pHead;
    
    for (int iCurrNode = 0; iCurrNode < g_LabelTable.iNodeCount; ++iCurrNode)
    {
        LabelNode * pCurrLabel = (LabelNode *) pCurrNode->pData;
        
        if (strcmp(pCurrLabel->pstrIdent, pstrIdent) == 0 && pCurrLabel->iFuncIndex == iFuncIndex)
            return pCurrLabel;
        
        pCurrNode = pCurrNode->pNext;
    }
    
    return NULL;
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
    for ( int iCurrInstrIndex = 0; iCurrInstrIndex < MAX_INSTR_LOOKUP_COUNT; ++ iCurrInstrIndex )
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

void ResetLexer ()
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
            
            ++ iIndex;
        }
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

void ShutDown ()
{
    // ---- Free source code array
    
    // Free each source line individually
    
    for ( int iCurrLineIndex = 0; iCurrLineIndex < g_iSourceCodeSize; ++ iCurrLineIndex )
        free ( g_ppstrSourceCode [ iCurrLineIndex ] );
    
    // Now free the base pointer
    
    free ( g_ppstrSourceCode );
    
    // ---- Free the assembled instruction stream
    
    if ( g_pInstrStream )
    {
        // Free each instruction's operand list
        
        for ( int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++ iCurrInstrIndex )
            if ( g_pInstrStream [ iCurrInstrIndex ].pOpList )
                free ( g_pInstrStream [ iCurrInstrIndex ].pOpList );
        
        // Now free the stream itself
        
        free ( g_pInstrStream );
    }
    
    // ---- Free the tables
    
    FreeLinkedList ( & g_SymbolTable );
    FreeLinkedList ( & g_LabelTable );
    FreeLinkedList ( & g_FuncTable );
    FreeLinkedList ( & g_StringTable );
    FreeLinkedList ( & g_HostAPICallTable );
}

void Exit ()
{
    ShutDown ();
    
    exit (0);
}

void ExitOnError (char * pstrErrorMssg)
{
    printf ("Fatal Error: %s.\n", pstrErrorMssg);
    
    Exit ();
}

void ExitOnCodeError (const char * pstrErrorMssg)
{
    printf ("Error: %s. \n\n", pstrErrorMssg);
    printf ("Line %d\n", g_Lexer.iCurrSourceLine);
    
    char pstrSourceLine[MAX_SOURCE_LINE_SIZE];
    strcpy (pstrSourceLine, g_ppstrSourceCode[g_Lexer.iCurrSourceLine]);
    
    for (unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen(pstrSourceLine);
         ++ iCurrCharIndex)
    {
        if (pstrSourceLine[iCurrCharIndex] == '\t')
            pstrSourceLine[iCurrCharIndex] = ' ';
    }
    
    printf ("%s", pstrSourceLine);
    
    for (unsigned int iCurrSpace = 0; iCurrSpace < g_Lexer.iIndex0; ++ iCurrSpace)
        printf (" ");
    printf ("^\n");
    
    printf ("Could not assemble %s.\n", g_pstrExecFilename);
    
    Exit();
}

void ExitOnCharExpectedError (char cChar)
{
    char * pstrErrorMssg = (char *) malloc (strlen ("' ' expected"));
    sprintf (pstrErrorMssg, "'%c' expected", cChar);
    
    ExitOnCodeError(pstrErrorMssg);
}

// ---- Main -----

int main (int argc, char * argv[])
{
    
}
