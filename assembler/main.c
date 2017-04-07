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

#define TRUE 1
#define FALSE 0

// ---- Functions ----
void AssmblSourceFile ()
{
    g_ScriptHeader.iStackSize = 0;
    g_ScriptHeader.iIsMainFuncPresent = FALSE;

    g_iInstrStreamSize = 0;
    g_iIsSetStackSizeFound = FALSE;
    g_ScriptHeader.iGlobalDataSize = 0;

    int iIsFuncActive = FALSE;
    FuncNode * pCurrFunc = NULL;
    int iCurrFuncIndex = 0;
    char pstrCurrFuncName[MAX_IDENT_SIZE];
    int iCurrFuncParamCount = 0;
    int iCurrFuncLocalDataSize = 0;

    InstrLookup CurrInstr;

    ResetLexer();

    while (TRUE)
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

                g_iIsSetStackSizeFound = TRUE;

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
                    g_ScriptHeader.iIsMainFuncPresent = TRUE;
                    g_ScriptHeader.iMainFuncIndex = iFuncIndex;
                }

                iIsFuncActive = TRUE;
                strcpy (pstrCurrFuncName, pstrFuncName);
                iCurrFuncIndex = iFuncIndex;
                iCurrFuncParamCount = 0;
                iCurrFuncLocalDataSize = 0;

                while (GetNextToken () == TOKEN_TYPE_NEWLINE) ;

                if (g_Lexer.CurrToken != TOKEN_TYPE_OPEN_BRACE)
                    ExitOnCharExpectedError ('{');

                ++g_iInstrStreamSize;

                break;
            }

            case TOKEN_TYPE_CLOSE_BRACE:
            {
                if (!iIsFuncActive)
                    ExitOnCharExpectedError ('}');

                SetFuncInfo (pstrCurrFuncName, iCurrFuncParamCount,
                             iCurrFuncLocalDataSize);

                iIsFuncActive = FALSE;
                break;
            }

            case TOKEN_TYPE_PARAM:
            {
                if (!iIsFuncActive)
                    ExitOnCodeError(ERROR_MSSG_GLOBAL_PARAM);

                if (strcmp(pstrCurrFuncName, MAIN_FUNC_NAME) == 0)
                    ExitOnCodeError(ERROR_MSSG_MAIN_PARAM);

                if (GetNextToken() != TOKEN_TYPE_IDENT)
                    ExitOnCodeError(ERROR_MSSG_IDENT_EXPECTED);
                ++iCurrFuncParamCount;
                break;
            }

            case TOKEN_TYPE_INSTR:
            {
                if (!iIsFuncActive)
                    ExitOnCodeError(ERROR_MSSG_GLOBAL_INSTR);

                ++g_iInstrStreamSize;
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

            default:
            {
                if (g_Lexer.CurrToken != TOKEN_TYPE_NEWLINE) {
                    //debug
//                    puts("first pass!");
//                    printf("%d\n", g_Lexer.CurrToken);
                    ExitOnCodeError(ERROR_MSSG_INVALID_INPUT);
                }
            }
        }

        // to next line
        if (!SkipToNextLine())
            break;
    }

    g_pInstrStream = (Instr * ) malloc(g_iInstrStreamSize * sizeof(Instr));

    for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++iCurrInstrIndex)
        g_pInstrStream[iCurrInstrIndex].pOpList = NULL;

    g_iCurrInstrIndex = 0;

    ResetLexer();

    while (TRUE)
    {
        if (GetNextToken() == END_OF_TOKEN_STREAM)
            break;

        switch (g_Lexer.CurrToken) {
            case TOKEN_TYPE_FUNC:
            {
                GetNextToken();

                pCurrFunc = GetFuncByName(GetCurrLexeme());

                iIsFuncActive = TRUE;

                iCurrFuncParamCount = 0;

                iCurrFuncIndex = pCurrFunc->iIndex;

                while (GetNextToken() == TOKEN_TYPE_NEWLINE) ;

                break;
            }

            case TOKEN_TYPE_CLOSE_BRACE:
            {
                iIsFuncActive = FALSE;
                if (strcmp(pCurrFunc->pstrName, MAIN_FUNC_NAME) == 0)
                {
                    g_pInstrStream[g_iCurrInstrIndex].iOpcode = INSTR_EXIT;
                    g_pInstrStream[g_iCurrInstrIndex].iOpCount = 1;

                    g_pInstrStream[g_iCurrInstrIndex].pOpList = (Op *) malloc(sizeof(Op));
                    g_pInstrStream[g_iCurrInstrIndex].pOpList[0].iType = OP_TYPE_INT;
                    g_pInstrStream[g_iCurrInstrIndex].pOpList[0].iIntLiteral = 0;
                }
                else
                {
                    g_pInstrStream[g_iCurrInstrIndex].iOpcode = INSTR_RET;
                    g_pInstrStream[g_iCurrInstrIndex].iOpCount = 0;
                    g_pInstrStream[g_iCurrInstrIndex].pOpList = NULL;
                }

                ++g_iCurrInstrIndex;

                break;
            }

            case TOKEN_TYPE_PARAM:
            {
                if (GetNextToken() != TOKEN_TYPE_IDENT)
                    ExitOnCodeError(ERROR_MSSG_IDENT_EXPECTED);

                char * pstrIdent = GetCurrLexeme();

                int iStackIndex = -(pCurrFunc->iLocalDataSize + 2 + (iCurrFuncParamCount + 1));

                if (AddSymbol(pstrIdent, 1, iStackIndex, iCurrFuncIndex) == -1)
                    ExitOnCodeError(ERROR_MSSG_IDENT_REDEFINITION);

                ++iCurrFuncParamCount;

                break;
            }

            case TOKEN_TYPE_INSTR:
            {
                GetInstrByMnemonic(GetCurrLexeme(), &CurrInstr);
                g_pInstrStream[g_iCurrInstrIndex].iOpcode = CurrInstr.iOpcode;
                g_pInstrStream[g_iCurrInstrIndex].iOpCount = CurrInstr.iOpCount;
                Op * pOpList = (Op *) malloc(CurrInstr.iOpCount * sizeof(Op));


                for (int iCurrOpIndex = 0; iCurrOpIndex < CurrInstr.iOpCount; ++iCurrOpIndex)
                {
                    OpTypes CurrOpTypes = CurrInstr.OpList[iCurrOpIndex];
                    Token InitOpToken = GetNextToken();
                    switch (InitOpToken)
                    {
                        case TOKEN_TYPE_INT:
                        {
                            if (CurrOpTypes & OP_FLAG_TYPE_INT)
                            {
                                pOpList[iCurrOpIndex].iType = OP_TYPE_INT;
                                pOpList[iCurrOpIndex].iIntLiteral = atoi(GetCurrLexeme());
                            }
                            else
                            {
                                ExitOnCodeError(ERROR_MSSG_INVALID_OP);
                            }
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

                                        int iStringIndex = AddString ( &g_StringTable, pstrString );

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

                                if ( !GetSymbolByIdent ( pstrIdent, iCurrFuncIndex ) )
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

                                        if ( !GetSymbolByIdent ( pstrIndexIdent, iCurrFuncIndex ) )
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

                                if ( !pLabel )
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

                                if ( !pFunc )
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

                                int iIndex = AddString ( &g_HostAPICallTable, pstrHostAPICall );

                                // Set the operand type to host API call index and set its
                                // data field

                                pOpList [ iCurrOpIndex ].iType = OP_TYPE_HOST_API_CALL_INDEX;
                                pOpList [ iCurrOpIndex ].iHostAPICallIndex = iIndex;
                            }

                            break;
                        }

                        // Anything else

                        default:
                        {
                            ExitOnCodeError ( ERROR_MSSG_INVALID_OP );
                            break;
                        }
                    }

                    // Make sure a comma follows the operand, unless it's the last one


                    if (iCurrOpIndex < CurrInstr.iOpCount - 1)
                    {
                        if (GetNextToken() != TOKEN_TYPE_COMMA)
                            ExitOnCharExpectedError(',');
                    }
                }

                if (GetNextToken() != TOKEN_TYPE_NEWLINE) {
//                    //debug
//                    puts("second pass!");
                    ExitOnCodeError(ERROR_MSSG_INVALID_INPUT);
                }

                g_pInstrStream[g_iCurrInstrIndex].pOpList = pOpList;

                ++g_iCurrInstrIndex;

            }
        }
    }

}

void strupr (char * pstrString)
{
    for (int iCurrCharIndex = 0;
         iCurrCharIndex < strlen (pstrString); ++iCurrCharIndex)
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

    if ( !pList->iNodeCount)
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
    if ( !pList )
        return;

    if ( pList->iNodeCount )
    {
        LinkedListNode * pCurrNode,
                       * pNextNode;

        pCurrNode = pList->pHead;

        while ( TRUE )
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

    for ( int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++iCurrNode )
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

    int iIndex = AddNode ( &g_FuncTable, pNewFunc );

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
    if ( !g_FuncTable.iNodeCount )
        return NULL;

    LinkedListNode * pCurrNode = g_FuncTable.pHead;

    for (int iCurrNode = 0; iCurrNode < g_FuncTable.iNodeCount; ++iCurrNode)
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

    int iIndex = AddNode ( &g_SymbolTable, pNewSymbol );

    pNewSymbol->iIndex = iIndex;

    return iIndex;
}

SymbolNode * GetSymbolByIdent ( char * pstrIdent, int iFuncIndex )
{
    if ( !g_SymbolTable.iNodeCount )
        return NULL;

    LinkedListNode * pCurrNode = g_SymbolTable.pHead;

    for ( int iCurrNode = 0; iCurrNode < g_SymbolTable.iNodeCount; ++iCurrNode )
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

int GetStackIndexByIdent ( char * pstrIdent, int iFuncIndex )
{
    SymbolNode * pSymbol = GetSymbolByIdent ( pstrIdent, iFuncIndex );

    return pSymbol->iStackIndex;
}

int GetSizeByIdent ( char * pstrIdent, int iFuncIndex )
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

    int iIndex = AddNode ( &g_LabelTable, pNewLabel );

    pNewLabel->iIndex = iIndex;

    return iIndex;
}

LabelNode * GetLabelByIndex ( char * pstrIdent, int iFuncIndex )
{
    if (!g_LabelTable.iNodeCount)
        return NULL;

    LinkedListNode * pCurrNode = g_LabelTable.pHead;

    for ( int iCurrNode = 0; iCurrNode < g_LabelTable.iNodeCount; ++iCurrNode )
    {
        LabelNode * pCurrLabel = ( LabelNode * ) pCurrNode->pData;

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

    g_InstrTable [ iInstrIndex ].OpList = ( OpTypes * ) malloc ( iOpCount * sizeof ( OpTypes ) );

    int iReturnInstrIndex = iInstrIndex;

    ++iInstrIndex;

    return iReturnInstrIndex;
}

void SetOpType ( int iInstrIndex, int iOpIndex, OpTypes iOpType )
{
    g_InstrTable [ iInstrIndex ].OpList [ iOpIndex] = iOpType;
}

int GetInstrByMnemonic ( char * pstrMnemonic, InstrLookup * pInstr )
{
    for ( int iCurrInstrIndex = 0; iCurrInstrIndex < MAX_INSTR_LOOKUP_COUNT; ++iCurrInstrIndex )
    {
        if (strlen(pstrMnemonic) > 0 && strcmp ( g_InstrTable [ iCurrInstrIndex ].pstrMnemonic, pstrMnemonic ) == 0 )
        {
            *pInstr = g_InstrTable [ iCurrInstrIndex ];
            return TRUE;
        }
    }

    return FALSE;
}

int SkipToNextLine ()
{
    ++g_Lexer.iCurrSourceLine;

    if (g_Lexer.iCurrSourceLine >= g_iSourceCodeSize)
        return FALSE;

    g_Lexer.iIndex0 = 0;
    g_Lexer.iIndex1 = 0;

    g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;

    return TRUE;
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
        while (TRUE)
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

            ++iIndex;
        }
    }

    return g_ppstrSourceCode[iCurrSourceLine][iIndex];

}


void StripComments (char * pstrSourceLine)
{
    int iCurrCharIndex;
    int iInString;

    iInString = 0;
    // Must check blank line!!
    int iLineLength = (int) strlen (pstrSourceLine);
    if (!iLineLength)
        return;

    for (iCurrCharIndex = 0; iCurrCharIndex < iLineLength; ++iCurrCharIndex)
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
        return TRUE;
    else
        return FALSE;
}

int IsCharNumeric (char cChar)
{
    if (cChar >= '0' && cChar <= '9')
        return TRUE;
    else
        return FALSE;
}

int IsCharIdent (char cChar)
{
    if ((cChar >= '0' && cChar <= '9') ||
        (cChar >= 'A' && cChar <= 'Z') ||
        (cChar >= 'a' && cChar <= 'z') ||
        cChar == '_')
        return TRUE;
    else
        return FALSE;
}

int IsCharDelimiter (char cChar)
{
    if (cChar == ':' || cChar == ',' || cChar == '"' ||
        cChar == '[' || cChar == ']' || cChar == '{' ||
        cChar == '}' || IsCharWhitespace (cChar) || cChar == '\n')
        return TRUE;
    else
        return FALSE;
}

void TrimWhitespace (char * pstrString)
{
    unsigned int iStringLength = (int) strlen (pstrString);
    unsigned int iPadLength;
    int iCurrCharIndex;

    if (iStringLength > 1)
    {
        for (iCurrCharIndex = 0; iCurrCharIndex < iStringLength; ++iCurrCharIndex)
            if (!IsCharWhitespace (pstrString[iCurrCharIndex]))
                break;

        iPadLength = iCurrCharIndex;
        if (iPadLength)
        {
            for (iCurrCharIndex = iPadLength; iCurrCharIndex < iStringLength;
                 ++iCurrCharIndex)
                pstrString[iCurrCharIndex - iPadLength] = pstrString[iCurrCharIndex];

            for (iCurrCharIndex = iStringLength - iPadLength;
                 iCurrCharIndex < iStringLength; ++iCurrCharIndex)
                pstrString[iCurrCharIndex] = ' ';
        }

        for (iCurrCharIndex = iStringLength - 1; iCurrCharIndex >= 0; --iCurrCharIndex)
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
        return FALSE;

    if (strlen (pstrString) == 0)
        return TRUE;

    for (unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString);
         ++iCurrCharIndex)
        if (!IsCharWhitespace (pstrString[iCurrCharIndex]) &&
            pstrString[iCurrCharIndex != '\n'])
            return FALSE;
    return TRUE;
}

int IsStringIdent (char * pstrString)
{
    if (!pstrString)
        return FALSE;

    if (strlen(pstrString) == 0)
        return FALSE;

    if (pstrString[0] >= '0' && pstrString[0] <= '9')
        return FALSE;

    for (unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString);
         ++iCurrCharIndex)
        if (!IsCharIdent(pstrString[iCurrCharIndex]))
            return FALSE;
    return TRUE;
}

int IsStringInteger (char * pstrString)
{
    if (!pstrString)
        return FALSE;

    if (strlen(pstrString) == 0)
        return FALSE;

    unsigned int iCurrCharIndex;

    for (iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString); ++iCurrCharIndex)
        if (!IsCharNumeric (pstrString[iCurrCharIndex]) && !(pstrString[iCurrCharIndex] == '-'))
            return FALSE;

    for (iCurrCharIndex = 1; iCurrCharIndex < strlen (pstrString); ++iCurrCharIndex)
        if (pstrString[iCurrCharIndex] == '-')
            return FALSE;

    return TRUE;
}


int IsStringFloat(char * pstrString)
{
    if (!pstrString)
        return FALSE;

    if (strlen (pstrString) == 0)
        return FALSE;

    unsigned int iCurrCharIndex;

    for (iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString); ++iCurrCharIndex)
        if (!IsCharNumeric (pstrString[iCurrCharIndex]) &&
            !(pstrString[iCurrCharIndex] == '.') &&
            !(pstrString[iCurrCharIndex] == '-'))
            return FALSE;

    int iRadixPointFound = 0;

    for (iCurrCharIndex = 0; iCurrCharIndex < strlen (pstrString); ++iCurrCharIndex)
    {
        if (pstrString[iCurrCharIndex] == '.')
        {
            if (iRadixPointFound)
                return FALSE;
            else
                iRadixPointFound = 1;
        }
    }

    for (iCurrCharIndex = 1; iCurrCharIndex < strlen (pstrString); ++iCurrCharIndex)
        if (pstrString[iCurrCharIndex] == '-')
            return FALSE;

    if (iRadixPointFound)
        return TRUE;
    else
        return FALSE;
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
        while (TRUE)
        {
            if (!IsCharWhitespace(g_ppstrSourceCode[g_Lexer.iCurrSourceLine][g_Lexer.iIndex0]))
                break;

            ++g_Lexer.iIndex0;
        }
    }

    g_Lexer.iIndex1 = g_Lexer.iIndex0;

    while (TRUE)
    {
        if (g_Lexer.iCurrLexState == LEX_STATE_IN_STRING)
        {
            if (g_Lexer.iIndex1 >= strlen (g_ppstrSourceCode[g_Lexer.iCurrSourceLine]))
            {
                //debug
//                puts("first invalid");
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

            ++g_Lexer.iIndex1;
        }

        else
        {
            if (g_Lexer.iIndex1 >= strlen (g_ppstrSourceCode[g_Lexer.iCurrSourceLine]))
                break;

            if (IsCharDelimiter (g_ppstrSourceCode[g_Lexer.iCurrSourceLine][g_Lexer.iIndex1]))
                break;

            ++g_Lexer.iIndex1;
        }
    }

    if (g_Lexer.iIndex1 - g_Lexer.iIndex0 == 0)
        ++g_Lexer.iIndex1;

    unsigned int iCurrDestIndex = 0;
    for (unsigned int iCurrSourceIndex = g_Lexer.iIndex0; iCurrSourceIndex < g_Lexer.iIndex1; ++iCurrSourceIndex)
    {
        if (g_Lexer.iCurrLexState == LEX_STATE_IN_STRING)
            if (g_ppstrSourceCode[g_Lexer.iCurrSourceLine][iCurrSourceIndex] == '\\')
                ++iCurrSourceIndex;

        g_Lexer.pstrCurrLexeme[iCurrDestIndex] = g_ppstrSourceCode[g_Lexer.iCurrSourceLine][iCurrSourceIndex];

        ++iCurrDestIndex;
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

    if (GetInstrByMnemonic(g_Lexer.pstrCurrLexeme, &Instr))
        g_Lexer.CurrToken = TOKEN_TYPE_INSTR;

//    printf("%s: %d\n", g_Lexer.pstrCurrLexeme, g_Lexer.CurrToken);
    return g_Lexer.CurrToken;
}

void ShutDown ()
{
    // ---- Free source code array

    // Free each source line individually

    for ( int iCurrLineIndex = 0; iCurrLineIndex < g_iSourceCodeSize; ++iCurrLineIndex )
        free ( g_ppstrSourceCode [ iCurrLineIndex ] );

    // Now free the base pointer

    free ( g_ppstrSourceCode );

    // ---- Free the assembled instruction stream

    if ( g_pInstrStream )
    {
        // Free each instruction's operand list

        for ( int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++iCurrInstrIndex )
            if ( g_pInstrStream [ iCurrInstrIndex ].pOpList )
                free ( g_pInstrStream [ iCurrInstrIndex ].pOpList );

        // Now free the stream itself

        free ( g_pInstrStream );
    }

    // ---- Free the tables

    FreeLinkedList ( &g_SymbolTable );
    FreeLinkedList ( &g_LabelTable );
    FreeLinkedList ( &g_FuncTable );
    FreeLinkedList ( &g_StringTable );
    FreeLinkedList ( &g_HostAPICallTable );
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
         ++iCurrCharIndex)
    {
        if (pstrSourceLine[iCurrCharIndex] == '\t')
            pstrSourceLine[iCurrCharIndex] = ' ';
    }

    printf ("%s", pstrSourceLine);

    for (unsigned int iCurrSpace = 0; iCurrSpace < g_Lexer.iIndex0; ++iCurrSpace)
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

void LoadSourceFile ()
{
    // Open the source file in binary mode

    if ( !( g_pSourceFile = fopen ( g_pstrSourceFilename, "rb" ) ) )
        ExitOnError ( "Could not open source file" );

    // Count the number of source lines

    while ( !feof ( g_pSourceFile ) )
        if ( fgetc ( g_pSourceFile ) == '\n' )
            ++g_iSourceCodeSize;
    ++g_iSourceCodeSize;

    // Close the file

    fclose ( g_pSourceFile );

    // Reopen the source file in ASCII mode

    if ( !( g_pSourceFile = fopen ( g_pstrSourceFilename, "r" ) ) )
        ExitOnError ( "Could not open source file" );

    // Allocate an array of strings to hold each source line

    g_ppstrSourceCode = ( char ** ) malloc ( g_iSourceCodeSize * sizeof ( char * ));
    if (!(g_ppstrSourceCode))
        ExitOnError ( "Could not allocate space for source code" );


    // Read the source code in from the file


    for ( int iCurrLineIndex = 0; iCurrLineIndex < g_iSourceCodeSize; ++iCurrLineIndex )
    {
        // Allocate space for the line
        g_ppstrSourceCode [ iCurrLineIndex ] = (char *) malloc(MAX_SOURCE_LINE_SIZE + 1);
        if (!g_ppstrSourceCode)
            ExitOnError ( "Could not allocate space for source line" );

        // Read in the current line

        fgets ( g_ppstrSourceCode [ iCurrLineIndex ], MAX_SOURCE_LINE_SIZE, g_pSourceFile );

        // Strip comments and trim whitespace

        StripComments ( g_ppstrSourceCode [ iCurrLineIndex ] );
        TrimWhitespace ( g_ppstrSourceCode [ iCurrLineIndex ] );

        // Make sure to add a new newline if it was removed by the stripping of the
        // comments and whitespace. We do this by checking the character right before
        // the null terminator to see if it's \n. If not, we move the terminator over
        // by one and add it. We use strlen () to find the position of the newline
        // easily.

//        int iNewLineIndex = (int) strlen ( g_ppstrSourceCode [ iCurrLineIndex ] ) - 1;
//        if ( g_ppstrSourceCode [ iCurrLineIndex ] [ iNewLineIndex ] != '\n' )
//        {
//            g_ppstrSourceCode [ iCurrLineIndex ] [ iNewLineIndex + 1 ] = '\n';
//            g_ppstrSourceCode [ iCurrLineIndex ] [ iNewLineIndex + 2 ] = '\0';
//        }
    }

//    puts("ok");
//
//    // debug
//    puts("debuging...");
//    for ( int iCurrLineIndex = 0; iCurrLineIndex < g_iSourceCodeSize; ++ iCurrLineIndex )
//    {
//        printf("%s", g_ppstrSourceCode[iCurrLineIndex]);
//    }
    //


    // Close the source file

    fclose ( g_pSourceFile );
}

void BuildXSE ()
{
    FILE * pExecFile;
    if (!(pExecFile = fopen(g_pstrExecFilename, "wb")))
        ExitOnCodeError(ERROR_MSSG_INVALID_FILE);

    fwrite (XSE_ID_STRING, 4, 2, pExecFile);

    char cVersionMajor = VERSION_MAJOR, cVersionMinor = VERSION_MINOR;

    fwrite(&cVersionMajor, 1, 1, pExecFile);
    fwrite(&cVersionMinor, 1, 1, pExecFile);

    fwrite(&g_ScriptHeader.iStackSize, 4, 1, pExecFile);
    fwrite(&g_ScriptHeader.iGlobalDataSize, 4, 1, pExecFile);

    char cIsMainPresent = 0;
    if (g_ScriptHeader.iIsMainFuncPresent)
        cIsMainPresent = 1;
    fwrite(&cIsMainPresent, 1, 1, pExecFile);
    fwrite(&g_ScriptHeader.iMainFuncIndex, 4, 1, pExecFile);

    fwrite(&g_iInstrStreamSize, 4, 1, pExecFile);

    for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++iCurrInstrIndex)
    {
        short sOpcode = g_pInstrStream[iCurrInstrIndex].iOpcode;
        fwrite(&sOpcode, 2, 1, pExecFile);

        char iOpCount = g_pInstrStream[iCurrInstrIndex].iOpCount;
        fwrite(&iOpCount, 1, 1, pExecFile);

        for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex) {
            Op CurrOp = g_pInstrStream[iCurrInstrIndex].pOpList[iCurrOpIndex];
            char cOpType = CurrOp.iType;
            fwrite(&cOpType, 1, 1, pExecFile);

            switch (CurrOp.iType) {
                // Integer literal

                case OP_TYPE_INT:
                    fwrite ( &CurrOp.iIntLiteral, sizeof ( int ), 1, pExecFile );
                    break;

                // Floating-point literal

                case OP_TYPE_FLOAT:
                    fwrite ( &CurrOp.fFloatLiteral, sizeof ( float ), 1, pExecFile );
                    break;

                // String index

                case OP_TYPE_STRING_INDEX:
                    fwrite ( &CurrOp.iStringTableIndex, sizeof ( int ), 1, pExecFile );
                    break;

                // Instruction index

                case OP_TYPE_INSTR_INDEX:
                    fwrite ( &CurrOp.iInstrIndex, sizeof ( int ), 1, pExecFile );
                    break;

                // Absolute stack index

                case OP_TYPE_ABS_STACK_INDEX:
                    fwrite ( &CurrOp.iStackIndex, sizeof ( int ), 1, pExecFile );
                    break;

                // Relative stack index

                case OP_TYPE_REL_STACK_INDEX:
                    fwrite ( &CurrOp.iStackIndex, sizeof ( int ), 1, pExecFile );
                    fwrite ( &CurrOp.iOffsetIndex, sizeof ( int ), 1, pExecFile );
                    break;

                // Function index

                case OP_TYPE_FUNC_INDEX:
                    fwrite ( &CurrOp.iFuncIndex, sizeof ( int ), 1, pExecFile );
                    break;

                // Host API call index

                case OP_TYPE_HOST_API_CALL_INDEX:
                    fwrite ( &CurrOp.iHostAPICallIndex, sizeof ( int ), 1, pExecFile );
                    break;

                // Register

                case OP_TYPE_REG:
                    fwrite ( &CurrOp.iReg, sizeof ( int ), 1, pExecFile );
                    break;
            }
        }

        int iCurrNode;
        LinkedListNode * pNode;
        char cParamCount;


        fwrite (&g_StringTable.iNodeCount, 4, 1, pExecFile);
        pNode = g_StringTable.pHead;

        for (iCurrNode = 0; iCurrNode < g_StringTable.iNodeCount; ++iCurrNode)
        {
            char * pstrCurrString = (char *) pNode->pData;
            int iCurrStringLength = (int) strlen(pstrCurrString);
            fwrite(&iCurrStringLength, 4, 1, pExecFile);
            fwrite(pstrCurrString, strlen(pstrCurrString), 1, pExecFile);

            pNode = pNode->pNext;
        }

        // ---- Write the function table

        // Write out the function count (4 bytes)

        fwrite ( &g_FuncTable.iNodeCount, 4, 1, pExecFile );

        // Set the pointer to the head of the list

        pNode = g_FuncTable.pHead;

        // Loop through each node in the list and write out its function info

        for ( iCurrNode = 0; iCurrNode < g_FuncTable.iNodeCount; ++iCurrNode )
        {
            // Create a local copy of the function

            FuncNode * pFunc = ( FuncNode * ) pNode->pData;

            // Write the entry point (4 bytes)

            fwrite ( &pFunc->iEntryPoint, sizeof ( int ), 1, pExecFile );

            // Write the parameter count (1 byte)

            cParamCount = pFunc->iParamCount;
            fwrite ( &cParamCount, 1, 1, pExecFile );

            // Write the local data size (4 bytes)

            fwrite ( &pFunc->iLocalDataSize, sizeof ( int ), 1, pExecFile );

            // Move to the next node

            pNode = pNode->pNext;
        }

        // ---- Write the host API call table

        // Write out the call count (4 bytes)

        fwrite ( &g_HostAPICallTable.iNodeCount, 4, 1, pExecFile );

        // Set the pointer to the head of the list

        pNode = g_HostAPICallTable.pHead;

        // Loop through each node in the list and write out its string

        for ( iCurrNode = 0; iCurrNode < g_HostAPICallTable.iNodeCount; ++iCurrNode )
        {
            // Copy the string pointer and calculate its length

            char * pstrCurrHostAPICall = ( char * ) pNode->pData;
            char cCurrHostAPICallLength = strlen ( pstrCurrHostAPICall );

            // Write the length (1 byte), followed by the string data (N bytes)

            fwrite ( &cCurrHostAPICallLength, 1, 1, pExecFile );
            fwrite ( pstrCurrHostAPICall, strlen ( pstrCurrHostAPICall ), 1, pExecFile );

            // Move to the next node

            pNode = pNode->pNext;
        }

        // ---- Close the output file

        fclose ( pExecFile );
    }
}

void InitInstrTable ()
{
    // Create a temporary index to use with each instruction

    int iInstrIndex;

    // The following code makes repeated calls to AddInstrLookup () to add a hardcoded
    // instruction set to the assembler's vocabulary. Each AddInstrLookup () call is
    // followed by zero or more calls to SetOpType (), whcih set the supported types of
    // a specific operand. The instructions are grouped by family.

    // ---- Main

    // Mov          Destination, Source

    iInstrIndex = AddInstrLookup ( "Mov", INSTR_MOV, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // ---- Arithmetic

    // Add         Destination, Source

    iInstrIndex = AddInstrLookup ( "Add", INSTR_ADD, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Sub          Destination, Source

    iInstrIndex = AddInstrLookup ( "Sub", INSTR_SUB, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Mul          Destination, Source

    iInstrIndex = AddInstrLookup ( "Mul", INSTR_MUL, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Div          Destination, Source

    iInstrIndex = AddInstrLookup ( "Div", INSTR_DIV, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Mod          Destination, Source

    iInstrIndex = AddInstrLookup ( "Mod", INSTR_MOD, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Exp          Destination, Source

    iInstrIndex = AddInstrLookup ( "Exp", INSTR_EXP, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Neg          Destination

    iInstrIndex = AddInstrLookup ( "Neg", INSTR_NEG, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Inc          Destination

    iInstrIndex = AddInstrLookup ( "Inc", INSTR_INC, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Dec          Destination

    iInstrIndex = AddInstrLookup ( "Dec", INSTR_DEC, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // ---- Bitwise

    // And          Destination, Source

    iInstrIndex = AddInstrLookup ( "And", INSTR_AND, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Or           Destination, Source

    iInstrIndex = AddInstrLookup ( "Or", INSTR_OR, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // XOr          Destination, Source

    iInstrIndex = AddInstrLookup ( "XOr", INSTR_XOR, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Not          Destination

    iInstrIndex = AddInstrLookup ( "Not", INSTR_NOT, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // ShL          Destination, Source

    iInstrIndex = AddInstrLookup ( "ShL", INSTR_SHL, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // ShR          Destination, Source

    iInstrIndex = AddInstrLookup ( "ShR", INSTR_SHR, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // ---- String Manipulation

    // Concat       String0, String1

    iInstrIndex = AddInstrLookup ( "Concat", INSTR_CONCAT, 2 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG |
                OP_FLAG_TYPE_STRING );

    // GetChar      Destination, Source, Index

    iInstrIndex = AddInstrLookup ( "GetChar", INSTR_GETCHAR, 3 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG |
                OP_FLAG_TYPE_STRING );
    SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG |
                OP_FLAG_TYPE_INT );

    // SetChar      Destination, Index, Source

    iInstrIndex = AddInstrLookup ( "SetChar", INSTR_SETCHAR, 3 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG |
                OP_FLAG_TYPE_INT );
    SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG |
                OP_FLAG_TYPE_STRING );

    // ---- Conditional Branching

    // Jmp          Label

    iInstrIndex = AddInstrLookup ( "Jmp", INSTR_JMP, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_LINE_LABEL );

    // JE           Op0, Op1, Label

    iInstrIndex = AddInstrLookup ( "JE", INSTR_JE, 3 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JNE          Op0, Op1, Label

    iInstrIndex = AddInstrLookup ( "JNE", INSTR_JNE, 3 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JG           Op0, Op1, Label

    iInstrIndex = AddInstrLookup ( "JG", INSTR_JG, 3 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JL           Op0, Op1, Label

    iInstrIndex = AddInstrLookup ( "JL", INSTR_JL, 3 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JGE          Op0, Op1, Label

    iInstrIndex = AddInstrLookup ( "JGE", INSTR_JGE, 3 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JLE           Op0, Op1, Label

    iInstrIndex = AddInstrLookup ( "JLE", INSTR_JLE, 3 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
    SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // ---- The Stack Interface

    // Push          Source

    iInstrIndex = AddInstrLookup ( "Push", INSTR_PUSH, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // Pop           Destination

    iInstrIndex = AddInstrLookup ( "Pop", INSTR_POP, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );

    // ---- The Function Interface

    // Call          FunctionName

    iInstrIndex = AddInstrLookup ( "Call", INSTR_CALL, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_FUNC_NAME );

    // Ret

    iInstrIndex = AddInstrLookup ( "Ret", INSTR_RET, 0 );

    // CallHost      FunctionName

    iInstrIndex = AddInstrLookup ( "CallHost", INSTR_CALLHOST, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_HOST_API_CALL );

    // ---- Miscellaneous

    // Pause        Duration

    iInstrIndex = AddInstrLookup ( "Pause", INSTR_PAUSE, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );


    // Exit         Code

    iInstrIndex = AddInstrLookup ( "Exit", INSTR_EXIT, 1 );
    SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                OP_FLAG_TYPE_FLOAT |
                OP_FLAG_TYPE_STRING |
                OP_FLAG_TYPE_MEM_REF |
                OP_FLAG_TYPE_REG );
}

void PrintLogo ()
{
    printf ( "XASM\n" );
    printf ( "XtremeScript Assembler Version %d.%d\n", VERSION_MAJOR, VERSION_MINOR );
    printf ( "\n" );
}

void PrintUsage ()
{
    printf ( "Usage:\tXASM Source.XASM [Executable.XSE]\n" );
    printf ( "\n" );
    printf ( "\t- File extensions are not required.\n" );
    printf ( "\t- Executable name is optional; source name is used by default.\n" );
}

void Init ()
{
    // Initialize the master instruction lookup table

    InitInstrTable ();

    // Initialize tables

    InitLinkedList ( &g_SymbolTable );
    InitLinkedList ( &g_LabelTable );
    InitLinkedList ( &g_FuncTable );
    InitLinkedList ( &g_StringTable );
    InitLinkedList ( &g_HostAPICallTable );
}

void PrintAssmblStats ()
{
    // ---- Calculate statistics

    // Symbols

    // Create some statistic variables

    int iVarCount = 0,
        iArrayCount = 0,
        iGlobalCount = 0;

    // Create a pointer to traverse the list

    LinkedListNode * pCurrNode = g_SymbolTable.pHead;

    // Traverse the list to count each symbol type

    for ( int iCurrNode = 0; iCurrNode < g_SymbolTable.iNodeCount; ++iCurrNode )
    {
        // Create a pointer to the current symbol structure

        SymbolNode * pCurrSymbol = ( SymbolNode * ) pCurrNode->pData;

        // It's an array if the size is greater than 1

        if ( pCurrSymbol->iSize > 1 )
            ++iArrayCount;

        // It's a variable otherwise

        else
            ++iVarCount;

        // It's a global if it's stack index is nonnegative

        if ( pCurrSymbol->iStackIndex >= 0 )
            ++iGlobalCount;

        // Move to the next node

        pCurrNode = pCurrNode->pNext;
    }

    // Print out final calculations

    printf ( "%s created successfully!\n\n", g_pstrExecFilename );
    printf ( "Source Lines Processed: %d\n", g_iSourceCodeSize );

    printf ( "            Stack Size: " );
    if ( g_ScriptHeader.iStackSize )
        printf ( "%d", g_ScriptHeader.iStackSize );
    else
        printf ( "Default" );

    printf ( "\n" );
    printf ( "Instructions Assembled: %d\n", g_iInstrStreamSize );
    printf ( "             Variables: %d\n", iVarCount );
    printf ( "                Arrays: %d\n", iArrayCount );
    printf ( "               Globals: %d\n", iGlobalCount );
    printf ( "       String Literals: %d\n", g_StringTable.iNodeCount );
    printf ( "                Labels: %d\n", g_LabelTable.iNodeCount );
    printf ( "        Host API Calls: %d\n", g_HostAPICallTable.iNodeCount );
    printf ( "             Functions: %d\n", g_FuncTable.iNodeCount );

    printf ( "      _Main () Present: " );
    if ( g_ScriptHeader.iIsMainFuncPresent )
        printf ( "Yes (Index %d)\n", g_ScriptHeader.iMainFuncIndex );
    else
        printf ( "No\n" );
}

// ---- Main -----

int main (int argc, char * argv[])
{


    // Print the logo

    PrintLogo ();

    // Validate the command line argument count

    if ( argc < 2 )
    {
        // If at least one filename isn't present, print the usage info and exit

        PrintUsage ();
        return 0;
    }

    // Before going any further, we need to validate the specified filenames. This may
    // include appending file extensions if they aren't present, and possibly copying the
    // source filename to the executable filename if the user didn't provide one.

    // First make a global copy of the source filename and convert it to uppercase

    strcpy ( g_pstrSourceFilename, argv [ 1 ] );
    strupr ( g_pstrSourceFilename );

    // Check for the presence of the .XASM extension and add it if it's not there

    if ( !strstr ( g_pstrSourceFilename, SOURCE_FILE_EXT ) )
    {
        // The extension was not found, so add it to string

        strcat ( g_pstrSourceFilename, SOURCE_FILE_EXT );
    }

    // Was an executable filename specified?

    if ( argv [ 2 ] )
    {
        // Yes, so repeat the validation process

        strcpy ( g_pstrExecFilename, argv [ 2 ] );
        strupr ( g_pstrExecFilename );

        // Check for the presence of the .XSE extension and add it if it's not there

        if ( !strstr ( g_pstrExecFilename, EXEC_FILE_EXT ) )
        {
            // The extension was not found, so add it to string

            strcat ( g_pstrExecFilename, EXEC_FILE_EXT );
        }
    }
    else
    {
        // No, so base it on the source filename

        // First locate the start of the extension, and use pointer subtraction to find the index

        int ExtOffset = (int) (strrchr ( g_pstrSourceFilename, '.' ) - g_pstrSourceFilename);
        strncpy ( g_pstrExecFilename, g_pstrSourceFilename, ExtOffset );

        // Append null terminator

        g_pstrExecFilename [ ExtOffset ] = '\0';

        // Append executable extension

        strcat ( g_pstrExecFilename, EXEC_FILE_EXT );
    }

    // Initialize the assembler

    Init ();

    // Load the source file into memory

    LoadSourceFile ();

    // Assemble the source file


    printf ( "Assembling %s...\n\n", g_pstrSourceFilename );

    AssmblSourceFile ();

    // Dump the assembled executable to an .XSE file

    BuildXSE ();

    // Print out assembly statistics

    PrintAssmblStats ();

    // Free resources and perform general cleanup

    ShutDown ();

    return 0;
}
