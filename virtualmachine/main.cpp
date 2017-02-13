/*
 *  main.cpp
 *  XVirtualMachine
 *
 *  Created by Taoran Xue on 1/30/17.
 *  Copyright © 2017 Taoran Xue. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* File I/O */
#define EXEC_FILE_EXT				".XSE"
#define XSE_ID_STRING               "XSE0"

/* LoadScript () Error Codes */

#define LOAD_OK						0
#define LOAD_ERROR_FILE_IO  	    1
#define LOAD_ERROR_INVALID_XSE		2
#define LOAD_ERROR_UNSUPPORTED_VERS	3

/* Operand Types */
#define OP_TYPE_NULL                -1          // Uninitialized/Null data
#define OP_TYPE_INT                 0           // Integer literal value
#define OP_TYPE_FLOAT               1           // Floating-point literal value
#define OP_TYPE_STRING		        2           // String literal value
#define OP_TYPE_ABS_STACK_INDEX     3           // Absolute array index
#define OP_TYPE_REL_STACK_INDEX     4           // Relative array index
#define OP_TYPE_INSTR_INDEX         5           // Instruction index
#define OP_TYPE_FUNC_INDEX          6           // Function index
#define OP_TYPE_HOST_API_CALL_INDEX 7           // Host API call index
#define OP_TYPE_REG                 8           // Register

/* Instruction Opcodes */
#define INSTR_MOV                   0

#define INSTR_ADD                   1
#define INSTR_SUB                   2
#define INSTR_MUL                   3
#define INSTR_DIV                   4
#define INSTR_MOD                   5
#define INSTR_EXP                   6
#define INSTR_NEG                   7
#define INSTR_INC                   8
#define INSTR_DEC                   9

#define INSTR_AND                   10
#define INSTR_OR                    11
#define INSTR_XOR                   12
#define INSTR_NOT                   13
#define INSTR_SHL                   14
#define INSTR_SHR                   15

#define INSTR_CONCAT                16
#define INSTR_GETCHAR               17
#define INSTR_SETCHAR               18

#define INSTR_JMP                   19
#define INSTR_JE                    20
#define INSTR_JNE                   21
#define INSTR_JG                    22
#define INSTR_JL                    23
#define INSTR_JGE                   24
#define INSTR_JLE                   25

#define INSTR_PUSH                  26
#define INSTR_POP                   27

#define INSTR_CALL                  28
#define INSTR_RET                   29
#define INSTR_CALLHOST              30

#define INSTR_PAUSE                 31
#define INSTR_EXIT                  32


#define DEF_STACK_SIZE			    1024	    // The default stack size

#define MAX_COERCION_STRING_SIZE    65          // The maximum allocated

/* Runtime Value */
typedef struct _Value
{
    int iType;
    union
    {
        int iIntLiteral;
        float fFloatLiteral;
        char * pstrStringLiteral;
        int iStackIndex;
        int iInstrIndex;
        int iFuncIndex;
        int iHostAPICallIndex;
        int iReg;
    };
    int iOffsetIndex;
}
Value;

/* An instruction */
typedef struct _Instr
{
    int iOpcode;
    int iOpCount;
    Value * pOpList;
}
Instr;

/* An instruction stream */
typedef struct _InstrStream
{
    Instr * pInstr;
    int iSize;
    int iCurrInstr;
}
InstrStream;

/* A runtime stack */
typedef struct _RuntimeStack
{
    Value * pElmnts;
    int iSize;
    int iTopIndex;
    int iFrameIndex;
}
RuntimeStack;

/* Function table element */
typedef struct _Func
{
    int iEntryPoint;
    int iParamCount;
    int iLocalDataSize;
    int iStackFrameSize;
}
Func;

/* A host API call table */
typedef struct _HostAPICallTable
{
    char ** ppstrCalls;
    int iSize;
}
HostAPICallTable;

/* Encapsulates a full script */
typedef struct _Script
{
    int iGlobalDataSize;
    int iIsMainFuncPresent;
    int iMainFuncIndex;
    int iIsPaused;
    int iPauseEndTime;
    
    /* Register file */
    Value _RetVal;
    
    /* Script data */
    InstrStream InstrStream;
    
    RuntimeStack Stack;
    Func * pFuncTable;
    HostAPICallTable HostAPICallTable;
}
Script;

Script g_Script;


/* Function declarations */
int LoadScript(char *);
int GetOpType(int);
int ResloveOpStackIndex(int);
Value ResolveOpValue(int);
Value GetStackValue(int);
int ResolveOpAsInt(int);
float ResolveOpAsFloat(int);
char * ResolveOpAsString(int);
char * CoerceValueToString(Value);

/* Function implements */
int LoadScript(char * pstrFilename)
{
    FILE * pScriptFile;
    if (!(pScriptFile = fopen(pstrFilename, "rb")))
        return LOAD_ERROR_FILE_IO;
    
    char * pstrIDString;
    pstrIDString = (char *) malloc (5);
    
    fread(pstrIDString, 4, 1, pScriptFile);
    pstrIDString[strlen(XSE_ID_STRING)] = '\0';
    
    if (strcmp(pstrIDString, XSE_ID_STRING) != 0)
        return LOAD_ERROR_INVALID_XSE;
    
    free(pstrIDString);
    
    int iMajorVersion = 0, iMinorVersion = 0;
    fread(&iMajorVersion, 1, 1, pScriptFile);
    fread(&iMinorVersion, 1, 1, pScriptFile);
    
    if (iMajorVersion != 0 || iMinorVersion != 4)
        return LOAD_ERROR_UNSUPPORTED_VERS;
    
    fread(&g_Script.Stack.iSize, 4, 1, pScriptFile);
    
    if (g_Script.Stack.iSize == 0)
        g_Script.Stack.iSize = DEF_STACK_SIZE;
    
    int iStackSize = g_Script.Stack.iSize;
    g_Script.Stack.pElmnts = (Value *) malloc(iStackSize * sizeof(Value));
    
    fread(&g_Script.iGlobalDataSize, 4, 1, pScriptFile);
    fread(&g_Script.iMainFuncIndex, 4, 1, pScriptFile);
    
    /* Read the instruction stream */
    fread(&g_Script.InstrStream.iSize, 4, 1, pScriptFile);
    
    g_Script.InstrStream.pInstr = (Instr *) malloc(g_Script.InstrStream.iSize * sizeof(Instr));
    
    for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Script.InstrStream.iSize; ++ iCurrInstrIndex)
    {
        g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpcode = 0;
        fread(&g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpcode, 2, 1, pScriptFile);
        
        g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpCount = 0;
        fread(&g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpCount, 1, 1, pScriptFile);
        
        int iOpCount = g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpCount;
        
        Value * pOpList;
        pOpList = (Value *) malloc(iOpCount * sizeof(Value));
        
        for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex)
        {
            pOpList[iCurrOpIndex].iType = 0;
            fread(&pOpList[iCurrOpIndex].iType, 1, 1, pScriptFile);
            
            switch (pOpList[iCurrOpIndex].iType) {
                case OP_TYPE_INT:
                    fread(&pOpList[iCurrOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
                    break;
                case OP_TYPE_FLOAT:
                    fread(&pOpList[iCurrOpIndex].fFloatLiteral, sizeof(float), 1, pScriptFile);
                    break;
                case OP_TYPE_STRING:
                    /* Since there is no filed in the Value structure for string table indices, read the
                     * index into the integer literal field and set its type to string index
                     */
                    fread(&pOpList[iCurrOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
                    break;
                case OP_TYPE_INSTR_INDEX:
                    fread(&pOpList[iCurrOpIndex].iInstrIndex, sizeof(int), 1, pScriptFile);
                    break;
                case OP_TYPE_ABS_STACK_INDEX:
                    fread(&pOpList[iCurrOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
                    break;
                case OP_TYPE_REL_STACK_INDEX:
                    fread(&pOpList[iCurrOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
                    fread(&pOpList[iCurrOpIndex].iOffsetIndex, sizeof(int), 1, pScriptFile);
                    break;
                case OP_TYPE_FUNC_INDEX:
                    fread(&pOpList[iCurrOpIndex].iFuncIndex, sizeof(int), 1, pScriptFile);
                    break;
                case OP_TYPE_HOST_API_CALL_INDEX:
                    fread(&pOpList[iCurrOpIndex].iHostAPICallIndex, sizeof(int), 1, pScriptFile);
                    break;
                case OP_TYPE_REG:
                    fread(&pOpList[iCurrOpIndex].iReg, sizeof(int), 1, pScriptFile);
                    break;
            }
        }
        g_Script.InstrStream.pInstr[iCurrInstrIndex].pOpList = pOpList;
    }
    
    /* Read the string table */
    int iStringTableSize;
    fread(&iStringTableSize, 4, 1, pScriptFile);
    
    if (iStringTableSize)
    {
        char ** ppstrStringTable = (char **) malloc(iStringTableSize * sizeof(char *));
        
        for (int iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++ iCurrStringIndex)
        {
            int iStringSize;
            fread(&iStringSize, 4, 1, pScriptFile);
            char * pstrCurrString = (char *) malloc(iStringSize + 1);
            
            fread(pstrCurrString, iStringSize, 1, pScriptFile);
            pstrCurrString[iStringSize] = '\0';
            
            ppstrStringTable[iCurrStringIndex] = pstrCurrString;
        }
        
        /* Run through each operand in the instruction stream and assign copies of string */
        for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Script.InstrStream.iSize; ++ iCurrInstrIndex)
        {
            int iOpCount = g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpCount;
            Value * pOpList = g_Script.InstrStream.pInstr[iCurrInstrIndex].pOpList;
            
            for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex)
            {
                if (pOpList[iCurrOpIndex].iType == OP_TYPE_STRING)
                {
                    int iStringIndex = pOpList[iCurrOpIndex].iIntLiteral;
                    char * pstrStringCopy = (char *) malloc(strlen(ppstrStringTable[iStringIndex]) + 1);
                    strcpy(pstrStringCopy, ppstrStringTable[iStringIndex]);
                    pOpList[iCurrOpIndex].pstrStringLiteral = pstrStringCopy;
                }
            }
        }
        
        /* Free the orginal strings */
        for (int iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++ iCurrStringIndex)
        {
            free(ppstrStringTable[iCurrStringIndex]);
        }
        free(ppstrStringTable);
        
        int iFuncTableSize;
        fread(&iFuncTableSize, 4, 1, pScriptFile);
        g_Script.pFuncTable = (Func *) malloc(iFuncTableSize * sizeof(Func));
        
        for (int iCurrFuncIndex = 0; iCurrFuncIndex < iFuncTableSize; ++ iCurrFuncIndex)
        {
            int iEntryPoint;
            fread(&iEntryPoint, 4, 1, pScriptFile);
            
            int iParamCount = 0;
            fread(&iParamCount, 1, 1, pScriptFile);
            
            int iLocalDataSize;
            fread(&iLocalDataSize, 4, 1, pScriptFile);
            
            int iStackFrameSize = iParamCount + 1 + iLocalDataSize;
            
            g_Script.pFuncTable[iCurrFuncIndex].iEntryPoint = iEntryPoint;
            g_Script.pFuncTable[iCurrFuncIndex].iParamCount = iParamCount;
            g_Script.pFuncTable[iCurrFuncIndex].iLocalDataSize = iLocalDataSize;
            g_Script.pFuncTable[iCurrFuncIndex].iStackFrameSize = iStackFrameSize;
        }
        
        /* Read the host API call table */
        
        fread(&g_Script.HostAPICallTable.iSize, 4, 1, pScriptFile);
        g_Script.HostAPICallTable.ppstrCalls = (char **) malloc(g_Script.HostAPICallTable.iSize * sizeof(char *));
        
        for (int iCurrCallIndex = 0; iCurrCallIndex < g_Script.HostAPICallTable.iSize; ++ iCurrCallIndex)
        {
            int iCallLength = 0;
            fread(&iCallLength, 1, 1, pScriptFile);
            
            char * pstrCurrCall = (char *) malloc(iCallLength + 1);
            
            fread(pstrCurrCall, iCallLength, 1, pScriptFile);
            pstrCurrCall[iCallLength] = '\0';
            
            g_Script.HostAPICallTable.ppstrCalls[iCurrCallIndex] = pstrCurrCall;
        }
        
        fclose ( pScriptFile );
        
        /* Print some information about the script */
        
        printf ( "%s loaded successfully!\n", pstrFilename );
        printf ( "\n" );
        printf ( "  Format Version: %d.%d\n", iMajorVersion, iMinorVersion );
        printf ( "      Stack Size: %d\n", g_Script.Stack.iSize );
        printf ( "Global Data Size: %d\n", g_Script.iGlobalDataSize );
        printf ( "       Functions: %d\n", iFuncTableSize );
        
        printf ( "_Main () Present: " );
        if (g_Script.iIsMainFuncPresent)
        {
            printf ( "Yes (Index %d)", g_Script.iMainFuncIndex );
        }
        else
        {
            printf ( "No" );
        }
        printf ( "\n" );
        
        printf ( "  Host API Calls: %d\n", g_Script.HostAPICallTable.iSize );
        printf ( "    Instructions: %d\n", g_Script.InstrStream.iSize );
        printf ( " String Literals: %d\n", iStringTableSize );
        
        printf ( "\n" );
        
    }
    
    return LOAD_OK;
}

/* Coerces a Value structure from it's current type to a string value. */
char * CoerceValueToString(Value Val)
{
    char * pstrCoercion;
    if (Val.iType != OP_TYPE_STRING)
        pstrCoercion = (char *) malloc(MAX_COERCION_STRING_SIZE + 1);
    
    switch (Val.iType) {
        case OP_TYPE_INT:
            sprintf(pstrCoercion, "%d", Val.iIntLiteral);
            return pstrCoercion;
        case OP_TYPE_FLOAT:
            sprintf(pstrCoercion, "%f", Val.fFloatLiteral);
            return pstrCoercion;
        case OP_TYPE_STRING:
            return Val.pstrStringLiteral;
        default:
            return NULL;
    }
}

/* Coerces a Value structure from it's current type to an integer value. */
int CoerceValueToInt(Value Val)
{
    switch (Val.iType) {
        case OP_TYPE_INT:
            return Val.iIntLiteral;
        case OP_TYPE_FLOAT:
            return (int) Val.fFloatLiteral;
        case OP_TYPE_STRING:
            return atoi(Val.pstrStringLiteral);
        default:
            return 0;
    }
}

float CoerceValueToFloat(Value Val)
{
    switch (Val.iType) {
        case OP_TYPE_INT:
            return (float) Val.iIntLiteral;
        case OP_TYPE_FLOAT:
            return Val.fFloatLiteral;
        case OP_TYPE_STRING:
            return atof(Val.pstrStringLiteral);
        default:
            return 0;
    }
}

int GetOpType(int iOpIndex)
{
    int iCurrInstr = g_Script.InstrStream.iCurrInstr;
    
    return g_Script.InstrStream.pInstr[iCurrInstr].pOpList[iOpIndex].iType;
}


int ResolveOpAsInt(int iOpIndex)
{
    Value OpValue = ResolveOpValue(iOpIndex);
    int iInt = CoerceValueToInt(OpValue);
    return iInt;
}

float ResolveOpAsFloat(int iOpIndex)
{
    Value OpValue = ResolveOpValue(iOpIndex);
    float fFloat = CoerceValueToFloat(OpValue);
    return fFloat;
}

char * ResolveOpAsString(int iOpIndex)
{
    Value OpValue = ResolveOpValue(iOpIndex);
    char * pstrString = CoerceValueToString(OpValue);
    return pstrString;
}

int ResloveOpStackIndex(int iOpIndex)
{
    return 0;
}

/* Resolves an operand and returns a pointer to it's Value structure */
Value * ResolveOpPntr(int iOpIndex)
{
    int iIndirMethod = GetOpType(iOpIndex);
    
    switch (iIndirMethod) {
        case OP_TYPE_REG:
            return &g_Script._RetVal;
        case OP_TYPE_ABS_STACK_INDEX:
        case OP_TYPE_REL_STACK_INDEX:
        {
            int iStackIndex = ResloveOpStackIndex(iOpIndex);
            return &g_Script.Stack.pElmnts[ResloveOpStackIndex(iStackIndex)];
        }
        default:
            return NULL;
    }
}

/* Resolves an operand and returns a pointer to it's Value structure */
Value ResolveOpValue(int iOpIndex)
{
    int iCurrInstr = g_Script.InstrStream.iCurrInstr;
    Value OpValue = g_Script.InstrStream.pInstr[iCurrInstr].pOpList[iOpIndex];
    
    switch (OpValue.iType) {
        case OP_TYPE_ABS_STACK_INDEX:
        case OP_TYPE_REL_STACK_INDEX:
        {
            int iAbsIndex = ResloveOpStackIndex(iOpIndex);
            return GetStackValue(iAbsIndex);
        }
        case OP_TYPE_REG:
            return g_Script._RetVal;
        default:
            return OpValue;
    }
}

Value GetStackValue(int iIndex)
{
    Value v;
    return v;
}

int main()
{
}
