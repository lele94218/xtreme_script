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
                    
                default:
                    break;
            }
        }
    }
    
    return LOAD_OK;
}

int main()
{
}
