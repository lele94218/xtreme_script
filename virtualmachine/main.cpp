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
#include <time.h>
#include <math.h>
#include <sys/select.h>

/* File I/O */
#define EXEC_FILE_EXT       ".XSE"
#define XSE_ID_STRING               "XSE0"

/* LoadScript () Error Codes */

#define LOAD_OK           0
#define LOAD_ERROR_FILE_IO        1
#define LOAD_ERROR_INVALID_XSE    2
#define LOAD_ERROR_UNSUPPORTED_VERS 3

/* Operand Types */
#define OP_TYPE_NULL                -1          // Uninitialized/Null data
#define OP_TYPE_INT                 0           // Integer literal value
#define OP_TYPE_FLOAT               1           // Floating-point literal value
#define OP_TYPE_STRING            2           // String literal value
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


#define DEF_STACK_SIZE          1024      // The default stack size

#define MAX_COERCION_STRING_SIZE    65          // The maximum allocated

/* ------------------------------ Data structure ------------------------------ */

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
    /* Index of the offset */
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
    /* Total size of the stack frame */
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

/* ------------------------------------ Global --------------------------------------- */
Script g_Script;
char ppstrMnemonics [][12] =
{
    "Mov",
    "Add", "Sub", "Mul", "Div", "Mod", "Exp", "Neg", "Inc", "Dec",
    "And", "Or", "XOr", "Not", "ShL", "ShR",
    "Concat", "GetChar", "SetChar",
    "Jmp", "JE", "JNE", "JG", "JL", "JGE", "JLE",
    "Push", "Pop",
    "Call", "Ret", "CallHost",
    "Pause", "Exit"
};


/* ------------------------------ Function declarations ------------------------------ */
int kbhit();
void ShutDown();
int LoadScript(char *);
int GetOpType(int);
int ResolveOpType(int);
int ResolveOpStackIndex(int);
char * ResolveOpAsHostAPICall(int);
Value ResolveOpValue(int);
Value GetStackValue(int);
int ResolveOpAsInt(int);
float ResolveOpAsFloat(int);
char * ResolveOpAsString(int);
int ResolveOpAsInstrIndex(int);
int ResolveOpAsFuncIndex(int);
char * CoerceValueToString(Value);
Value GetStackValue(int);
void SetStackValue(int, Value);
void Push(Value);
Value Pop();
void PushFrame(int);
void PopFrame(int);
Func GetFunc(int);
void CopyValue(Value *, Value);
Value * ResolveOpPntr(int);
void Runscript();
int GetCurrTime();
void PrintOpIndir(int);
void PrintOpValue(int);

/* ------------------------------------- Macros -------------------------------------- */

#define ResolveStackIndex(iIndex) \
    (iIndex < 0 ? iIndex += g_Script.Stack.iFrameIndex : iIndex)

/* ------------------------------ Function implements -------------------------------- */
int kbhit()
{
    struct timeval tv;
    fd_set read_fd;

    /* Do not wait at all, not even a microsecond */
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    /* Must be done first to initialize read_fd */
    FD_ZERO(&read_fd);

    /* Makes select() ask if input is ready:
     * 0 is the file descriptor for stdin    */
    FD_SET(0,&read_fd);

    /* The first parameter is the number of the
     * largest file descriptor to check + 1. */
    if(select(1, &read_fd, NULL, NULL, &tv) == -1)
        return 0;         /* An error occured */

    /*  read_fd now holds a bit map of files that are
     * readable. We test the entry for the standard
     * input (file 0). */

    if(FD_ISSET(0,&read_fd))
        /* Character pending on stdin */
        return 1;

    /* no characters were pending */
    return 0;
}

void ShutDown()
{
    /* Free instruction stream */
    for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Script.InstrStream.iSize; ++ iCurrInstrIndex)
    {
        int iOpCount = g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpCount;
        Value * pOpList = g_Script.InstrStream.pInstr[iCurrInstrIndex].pOpList;
        
        for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex)
        {
            if (pOpList[iCurrOpIndex].pstrStringLiteral)
                free(pOpList[iCurrOpIndex].pstrStringLiteral);
        }
        
        if (g_Script.InstrStream.pInstr)
            free(g_Script.InstrStream.pInstr);
    }
    
    for (int iCurrElmtnIndex = 0; iCurrElmtnIndex < g_Script.Stack.iSize; ++ iCurrElmtnIndex)
    {
        if (g_Script.Stack.pElmnts[iCurrElmtnIndex].iType == OP_TYPE_STRING)
        {
            free(g_Script.Stack.pElmnts[iCurrElmtnIndex].pstrStringLiteral);
        }
    }
    
    if (g_Script.Stack.pElmnts)
        free(g_Script.Stack.pElmnts);
    
    if (g_Script.pFuncTable)
        free(g_Script.pFuncTable);
    
    for (int iCurrCallIndex = 0; iCurrCallIndex < g_Script.HostAPICallTable.iSize; ++ iCurrCallIndex)
    {
        if (g_Script.HostAPICallTable.ppstrCalls[iCurrCallIndex])
            free(g_Script.HostAPICallTable.ppstrCalls[iCurrCallIndex]);
    }
    
    if (g_Script.HostAPICallTable.ppstrCalls)
        free(g_Script.HostAPICallTable.ppstrCalls);
}

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

    for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Script.InstrStream.iSize; ++iCurrInstrIndex)
    {
        g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpcode = 0;
        fread(&g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpcode, 2, 1, pScriptFile);

        g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpCount = 0;
        fread(&g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpCount, 1, 1, pScriptFile);

        int iOpCount = g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpCount;

        Value * pOpList;
        pOpList = (Value *) malloc(iOpCount * sizeof(Value));

        for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex)
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

        for (int iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++iCurrStringIndex)
        {
            int iStringSize;
            fread(&iStringSize, 4, 1, pScriptFile);
            char * pstrCurrString = (char *) malloc(iStringSize + 1);

            fread(pstrCurrString, iStringSize, 1, pScriptFile);
            pstrCurrString[iStringSize] = '\0';

            ppstrStringTable[iCurrStringIndex] = pstrCurrString;
        }

        /* Run through each operand in the instruction stream and assign copies of string */
        for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Script.InstrStream.iSize; ++iCurrInstrIndex)
        {
            int iOpCount = g_Script.InstrStream.pInstr[iCurrInstrIndex].iOpCount;
            Value * pOpList = g_Script.InstrStream.pInstr[iCurrInstrIndex].pOpList;

            for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex)
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
        for (int iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++iCurrStringIndex)
        {
            free(ppstrStringTable[iCurrStringIndex]);
        }
        free(ppstrStringTable);

        int iFuncTableSize;
        fread(&iFuncTableSize, 4, 1, pScriptFile);
        g_Script.pFuncTable = (Func *) malloc(iFuncTableSize * sizeof(Func));

        for (int iCurrFuncIndex = 0; iCurrFuncIndex < iFuncTableSize; ++iCurrFuncIndex)
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

        for (int iCurrCallIndex = 0; iCurrCallIndex < g_Script.HostAPICallTable.iSize; ++iCurrCallIndex)
        {
            int iCallLength = 0;
            fread(&iCallLength, 1, 1, pScriptFile);

            char * pstrCurrCall = (char *) malloc(iCallLength + 1);

            fread(pstrCurrCall, iCallLength, 1, pScriptFile);
            pstrCurrCall[iCallLength] = '\0';

            g_Script.HostAPICallTable.ppstrCalls[iCurrCallIndex] = pstrCurrCall;
        }

        fclose (pScriptFile);

        /* Print some information about the script */

        printf ("%s loaded successfully!\n", pstrFilename);
        printf ("\n");
        printf ("  Format Version: %d.%d\n", iMajorVersion, iMinorVersion);
        printf ("      Stack Size: %d\n", g_Script.Stack.iSize);
        printf ("Global Data Size: %d\n", g_Script.iGlobalDataSize);
        printf ("       Functions: %d\n", iFuncTableSize);

        printf ("_Main () Present: ");
        if (g_Script.iIsMainFuncPresent)
        {
            printf ("Yes (Index %d)", g_Script.iMainFuncIndex);
        }
        else
        {
            printf ("No");
        }
        printf ("\n");

        printf ("  Host API Calls: %d\n", g_Script.HostAPICallTable.iSize);
        printf ("    Instructions: %d\n", g_Script.InstrStream.iSize);
        printf (" String Literals: %d\n", iStringTableSize);

        printf ("\n");

    }

    return LOAD_OK;
}

/* Runs the currenty load script until a key is pressed or the script exits. */
void RunScript()
{
    int iExitExecLoop = false;

    while (!kbhit())
    {
        if (g_Script.iIsPaused)
        {
            if (GetCurrTime() >= g_Script.iPauseEndTime)
            {
                g_Script.iIsPaused = false;
            }
            else
            {
                continue;
            }
        }

        int iCurrInstr = g_Script.InstrStream.iCurrInstr;

        int iOpcode = g_Script.InstrStream.pInstr[iCurrInstr].iOpcode;

        /* Print some info about the instruction */

        printf("\t");

        if (iOpcode < 10)
            printf(" %d", iOpcode);
        else
            printf("%d", iOpcode);
        printf(" %s ", ppstrMnemonics[iOpcode]);

        switch(iOpcode) {
            /* Binary operations */
            case INSTR_MOV:
            case INSTR_ADD:
            case INSTR_SUB:
            case INSTR_MUL:
            case INSTR_DIV:
            case INSTR_MOD:
            case INSTR_EXP:
            case INSTR_AND:
            case INSTR_OR:
            case INSTR_XOR:
            case INSTR_SHL:
            case INSTR_SHR:
            {
                Value Dest = ResolveOpValue(0);
                Value Source = ResolveOpValue(1);

                switch (iOpcode)
                {
                    case INSTR_MOV:
                        /* Skip when the two operands are the same */
                        if (ResolveOpPntr(0) == ResolveOpPntr(1))
                            break;
                        CopyValue(&Dest, Source);
                        break;

                    case INSTR_ADD:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral += ResolveOpAsInt(1);
                        else
                            Dest.fFloatLiteral += ResolveOpAsFloat(1);
                        break;

                    case INSTR_SUB:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral -= ResolveOpAsInt(1);
                        else
                            Dest.fFloatLiteral -= ResolveOpAsFloat(1);
                        break;

                    case INSTR_MUL:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral *= ResolveOpAsInt(1);
                        else
                            Dest.fFloatLiteral *= ResolveOpAsFloat(1);
                        break;

                    case INSTR_DIV:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral /= ResolveOpAsInt(1);
                        else
                            Dest.fFloatLiteral /= ResolveOpAsFloat(1);

                        break;

                    case INSTR_MOD:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral %= ResolveOpAsInt(1);

                        break;

                    case INSTR_EXP:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral = (int) pow(Dest.iIntLiteral, ResolveOpAsInt(1));
                        else
                            Dest.fFloatLiteral = (float) pow(Dest.fFloatLiteral, ResolveOpAsFloat(1));

                        break;

                    case INSTR_AND:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral &= ResolveOpAsInt(1);

                        break;

                    case INSTR_OR:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral |= ResolveOpAsInt(1);

                        break;

                    case INSTR_XOR:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral ^= ResolveOpAsInt(1);

                        break;

                    case INSTR_SHL:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral <<= ResolveOpAsInt(1);

                        break;

                    case INSTR_SHR:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral >>= ResolveOpAsInt(1);

                        break;
                }

                *ResolveOpPntr(0) = Dest;

                    PrintOpIndir(0);
                printf(", ");
                PrintOpIndir(1);
                break;
            }
            /* Unary Operation */

            case INSTR_NEG:
            case INSTR_NOT:
            case INSTR_INC:
            case INSTR_DEC:
            {
                int iDestStoreType = GetOpType (0);
                Value Dest = ResolveOpValue (0);

                switch (iOpcode)
                {
                    case INSTR_NEG:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral = -Dest.iIntLiteral;
                        else
                            Dest.fFloatLiteral = -Dest.fFloatLiteral;

                        break;

                    case INSTR_NOT:

                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral = ~Dest.iIntLiteral;

                        break;

                    case INSTR_INC:

                        if (Dest.iType == OP_TYPE_INT)
                            ++Dest.iIntLiteral;
                        else
                            ++Dest.fFloatLiteral;

                        break;

                    case INSTR_DEC:

                        if (Dest.iType == OP_TYPE_INT)
                            --Dest.iIntLiteral;
                        else
                            --Dest.fFloatLiteral;

                        break;
                }

                *ResolveOpPntr(0) = Dest;
                PrintOpIndir(0);
                break;
            }

            /* String processing */
            case INSTR_CONCAT:
            {
                Value Dest = ResolveOpValue(0);
                char * pstrSourceString = ResolveOpAsString(1);

                if (Dest.iType != OP_TYPE_STRING)
                    break;

                int iNewStringLength = (int) strlen(Dest.pstrStringLiteral) +  (int) strlen(pstrSourceString);
                char * pstrNewString = (char *) malloc (iNewStringLength + 1);

                strcpy(pstrNewString, Dest.pstrStringLiteral);
                strcat(pstrNewString, pstrSourceString);
                free(Dest.pstrStringLiteral);
                Dest.pstrStringLiteral = pstrNewString;

                *ResolveOpPntr(0) = Dest;

                PrintOpIndir(0);
                printf(", ");
                PrintOpValue(1);
                break;
            }
            case INSTR_GETCHAR:
            {
                Value Dest = ResolveOpValue (0);
                // Get a local copy of the source string (operand index 1)

                char * pstrSourceString = ResolveOpAsString (1);

                char * pstrNewString;
                if (Dest.iType == OP_TYPE_STRING)
                {

                    if (strlen (Dest.pstrStringLiteral) >= 1)
                    {
                        pstrNewString = Dest.pstrStringLiteral;
                    }
                    else
                    {
                        free (Dest.pstrStringLiteral);
                        pstrNewString = ( char * ) malloc (2);
                    }
                }
                else
                {

                    pstrNewString = ( char * ) malloc (2);
                    Dest.iType = OP_TYPE_STRING;
                }

                int iSourceIndex = ResolveOpAsInt (2);

                pstrNewString [ 0 ] = pstrSourceString [ iSourceIndex ];
                pstrNewString [ 1 ] = '\0';

                Dest.pstrStringLiteral = pstrNewString;

                *ResolveOpPntr (0) = Dest;

                PrintOpIndir (0);
                printf (", ");
                PrintOpValue (1);
                printf (", ");
                PrintOpValue (2);
                break;
            }

            case INSTR_SETCHAR:
            {
                int iDestIndex = ResolveOpAsInt (1);

                if (ResolveOpType (0) != OP_TYPE_STRING)
                    break;

                char * pstrSourceString = ResolveOpAsString (2);

                ResolveOpPntr (0)->pstrStringLiteral [ iDestIndex ] = pstrSourceString [ 0 ];

                PrintOpIndir (0);
                printf (", ");
                PrintOpValue (1);
                printf (", ");
                PrintOpValue (2);
                break;
            }
                
            /* Conditional braching */
            case INSTR_JMP:
            {
                int iTargetIndex = ResolveOpAsInstrIndex(0);
                g_Script.InstrStream.iCurrInstr = iTargetIndex;
                
                PrintOpValue(0);
                break;
            }
            
            case INSTR_JE:
            case INSTR_JNE:
            case INSTR_JG:
            case INSTR_JL:
            case INSTR_JGE:
            case INSTR_JLE:
            {
                Value Op0 = ResolveOpValue(0);
                Value Op1 = ResolveOpValue(1);
                
                int iTargetIndex = ResolveOpAsInstrIndex(2);
                
                int iJump = false;
                
                switch (iOpcode) {
                    case INSTR_JE:
                    {
                        switch (Op0.iType) {
                            case OP_TYPE_INT:
                                if (Op0.iIntLiteral == Op1.iIntLiteral)
                                    iJump = true;
                                break;
                            
                            case OP_TYPE_FLOAT:
                                if (Op0.fFloatLiteral == Op1.fFloatLiteral)
                                    iJump = true;
                                break;
                                
                            case OP_TYPE_STRING:
                                if (strcmp(Op0.pstrStringLiteral, Op1.pstrStringLiteral) == 0)
                                    iJump = true;
                                break;
                        }
                        break;
                    }
                    
                    case INSTR_JNE:
                    {
                        switch (Op0.iType) {
                            case OP_TYPE_INT:
                                if (Op0.iIntLiteral != Op1.iIntLiteral)
                                    iJump = true;
                                break;
                                
                            case OP_TYPE_FLOAT:
                                if (Op0.fFloatLiteral != Op1.fFloatLiteral)
                                    iJump = true;
                                break;
                                
                            case OP_TYPE_STRING:
                                if (strcmp(Op0.pstrStringLiteral, Op1.pstrStringLiteral) != 0)
                                    iJump = true;
                                break;
                        }
                        break;
                    }
                        
                    case INSTR_JG:
                    {
                        if (Op0.iType == OP_TYPE_INT)
                        {
                            if (Op0.iIntLiteral > Op1.iIntLiteral)
                                iJump = true;
                        }
                        else
                        {
                            if (Op0.fFloatLiteral > Op1.fFloatLiteral)
                                iJump = true;
                        }
                        break;
                    }
                        
                    case INSTR_JL:
                    {
                        if (Op0.iType == OP_TYPE_INT)
                        {
                            if (Op0.iIntLiteral < Op1.iIntLiteral)
                                iJump = true;
                        }
                        else
                        {
                            if (Op0.fFloatLiteral < Op1.fFloatLiteral)
                                iJump = true;
                        }
                        break;
                    }
                        
                    case INSTR_JGE:
                    {
                        if (Op0.iType == OP_TYPE_INT)
                        {
                            if (Op0.iIntLiteral >= Op1.iIntLiteral)
                                iJump = true;
                        }
                        else
                        {
                            if (Op0.fFloatLiteral >= Op1.fFloatLiteral)
                                iJump = true;
                        }
                        break;
                    }
                    
                    case INSTR_JLE:
                    {
                        if (Op0.iType == OP_TYPE_INT)
                        {
                            if (Op0.iIntLiteral <= Op1.iIntLiteral)
                                iJump = true;
                        }
                        else
                        {
                            if (Op0.fFloatLiteral <= Op1.fFloatLiteral)
                                iJump = true;
                        }
                        break;
                    }
                }
                PrintOpValue(0);
                printf(", ");
                PrintOpValue(1);
                printf(", ");
                PrintOpValue(2);
                printf(" ");
                
                if (iJump)
                {
                    g_Script.InstrStream.iCurrInstr = iTargetIndex;
                    printf("(True)");
                }
                else
                {
                    printf("(False)");
                }
                break;
            }
            
            /* Stack interface */
            
            case INSTR_PUSH:
            {
                Value Source = ResolveOpValue(0);
                Push(Source);
                
                PrintOpValue(0);
                break;
            }
                
            case INSTR_POP:
            {
                * ResolveOpPntr(0) = Pop();
                
                PrintOpIndir(0);
                break;
            }
                
            case INSTR_CALL:
            {
                int iFuncIndex = ResolveOpAsFuncIndex(0);
                Func Dest = GetFunc(iFuncIndex);
                
                Value ReturnAddr;
                ReturnAddr.iInstrIndex = g_Script.InstrStream.iCurrInstr + 1;
                Push(ReturnAddr);
                /*
                 * Push the stack frame + 1 (the extra space is for the function index)
                 * we will put on the stack after it
                 */
                PushFrame(Dest.iLocalDataSize + 1);
                
                /* Write the function index to the top of the stack */
                
                Value FuncIndex;
                FuncIndex.iFuncIndex = iFuncIndex;
                SetStackValue(g_Script.Stack.iTopIndex - 1, FuncIndex);
                
                /* Make the jump to the function's entry point */
                g_Script.InstrStream.iCurrInstr = Dest.iEntryPoint;
                printf("$$[%d]$$", g_Script.InstrStream.iCurrInstr);
                printf("%d (Entry Point: %d, Frame Size %d)", iFuncIndex, Dest.iEntryPoint, Dest.iStackFrameSize);
                break;
            }
            
            case INSTR_RET:
            {
                /* 
                  * Get the current function index off the top of the stack and
                  * use it to get the corresponding function structure
                  */
                Value FuncIndex = Pop();
                Func CurrFunc = GetFunc(FuncIndex.iFuncIndex);
                int iFrameIndex = FuncIndex.iOffsetIndex;
                
                /*
                  * Read the return address structure from the stack, which is
                  * stored one index below the local data
                  */
                Value ReturnAddr = GetStackValue(g_Script.Stack.iTopIndex - (CurrFunc.iLocalDataSize + 1));
                
                /* Pop the stack frame along with the return address */
                PopFrame(CurrFunc.iStackFrameSize);
                
                /* Restore the previous frame index */
                g_Script.Stack.iFrameIndex = iFrameIndex;
                
                /* Jump to the return address */
                g_Script.InstrStream.iCurrInstr = ReturnAddr.iInstrIndex;
                
                printf("%d", ReturnAddr.iInstrIndex);
                break;
            }
                
            case INSTR_CALLHOST:
            {
                /* TODO */
                PrintOpValue(0);
                break;
            }
                
            case INSTR_PAUSE:
            {
                if (g_Script.iIsPaused)
                    break;
                
                /* Get the pause duration */
                int iPauseDuration = ResolveOpAsInt(0);
                
                /* Determine the ending pause time */
                g_Script.iPauseEndTime = GetCurrTime() + iPauseDuration;
                
                /* Pause the script */
                g_Script.iIsPaused = true;
                
                PrintOpValue(0);
                break;
            }
                
            case INSTR_EXIT:
            {
                Value ExitCode = ResolveOpValue(0);
                
                int iExitCode = ExitCode.iIntLiteral;
                
                iExitExecLoop = true;
                
                PrintOpValue(0);
                break;
            }
                
            
        }
            printf("\n");

            /* If the instruction pointer hasn't been changed by an instruction.*/
            if (iCurrInstr == g_Script.InstrStream.iCurrInstr)
                ++g_Script.InstrStream.iCurrInstr;

            if (iExitExecLoop)
                break;
    }
}

/* Reset the script */
void ResetScript()
{
    int iMainFuncIndex = g_Script.iMainFuncIndex;

    /* If the function table is present, set the entry point */
    if (g_Script.pFuncTable)
    {
        if (g_Script.iIsMainFuncPresent)
        {
            g_Script.InstrStream.iCurrInstr = g_Script.pFuncTable[iMainFuncIndex].iEntryPoint;
        }
    }

    /* Clear the stack */
    for (int iCurrElmntIndex = 0; iCurrElmntIndex < g_Script.Stack.iSize; ++iCurrElmntIndex)
    {
        g_Script.Stack.pElmnts[iCurrElmntIndex].iType = OP_TYPE_NULL;
    }

    /* Unpause the script */
    g_Script.iIsPaused = false;

    /* Allocate space for the globals */
    PushFrame(g_Script.iGlobalDataSize);

    /* If _Main() is present, push it's stack frame (plus one extra stack element to
     * compensate for the function index that usually sits on top of stack frames and
     * causes indeices to start from -2)
     */
    PushFrame(g_Script.pFuncTable[iMainFuncIndex].iStackFrameSize + 1);

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

/* Returns the type of the specified operand in the current instruction. */
int GetOpType(int iOpIndex)
{
    int iCurrInstr = g_Script.InstrStream.iCurrInstr;

    return g_Script.InstrStream.pInstr[iCurrInstr].pOpList[iOpIndex].iType;
}

/* Resolves the type of the specified operand in the current instruction and returns the
 * resolved type
 */
int ResolveOpType(int iOpIndex)
{
    Value OpValue = ResolveOpValue(iOpIndex);
    return OpValue.iType;
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
/*
 *	Resolves and coerces an operand's value to a string value, allocating the space for a
 *  new string if necessary.
 */
char * ResolveOpAsString(int iOpIndex)
{
    Value OpValue = ResolveOpValue(iOpIndex);
    char * pstrString = CoerceValueToString(OpValue);
    return pstrString;
}

int ResolveOpStackIndex(int iOpIndex)
{
    return 0;
}

/* Resolves an operand as a host API call */
char * ResolveOpAsHostAPICall(int iOpIndex)
{
    return NULL;
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
            int iStackIndex = ResolveOpStackIndex(iOpIndex);
            return &g_Script.Stack.pElmnts[ResolveOpStackIndex(iStackIndex)];
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
            int iAbsIndex = ResolveOpStackIndex(iOpIndex);
            return GetStackValue(iAbsIndex);
        }
        case OP_TYPE_REG:
            return g_Script._RetVal;
        default:
            return OpValue;
    }
}

/* Resolves an operand as an instruction index */
int ResolveOpAsInstrIndex(int iOpIndex)
{
    return 0;
}

/* Resolves an operand as a function index */
int ResolveOpAsFuncIndex(int iOpIndex)
{
    Value OpValue = ResolveOpValue(iOpIndex);
    return OpValue.iFuncIndex;
}

Value GetStackValue(int iIndex)
{
    return g_Script.Stack.pElmnts[ResolveStackIndex(iIndex)];
}

void SetStackValue(int iIndex, Value Val)
{
    g_Script.Stack.pElmnts[ResolveStackIndex(iIndex)] = Val;
}

void Push(Value Val)
{
    int iTopIndex = g_Script.Stack.iTopIndex;
    g_Script.Stack.pElmnts[iTopIndex] = Val;
    ++g_Script.Stack.iTopIndex;
}

Value Pop()
{
    --g_Script.Stack.iTopIndex;
    int iTopIndex = g_Script.Stack.iTopIndex;
    Value Val = g_Script.Stack.pElmnts[iTopIndex];
    return Val;
}

void PushFrame(int iSize)
{
    g_Script.Stack.iTopIndex += iSize;
    g_Script.Stack.iFrameIndex = g_Script.Stack.iTopIndex;
}

void PopFrame(int iSize)
{
    g_Script.Stack.iTopIndex -= iSize;
}

/* Copies a value structure to another, taking string into account. */
void CopyValue(Value * pDest, Value Source)
{
    /* If the destination already contains a string, make sure to free it first */
    if (pDest->iType == OP_TYPE_STRING)
    {
        free(pDest->pstrStringLiteral);
    }

    /* Copyt the objext */
    *pDest = Source;

    if (Source.iType == OP_TYPE_STRING)
    {
        pDest->pstrStringLiteral = (char *) malloc(strlen(Source.pstrStringLiteral) + 1);
        strcpy(pDest->pstrStringLiteral, Source.pstrStringLiteral);
    }

}

/* Returns the function corresponding to the specified index. */
Func GetFunc(int iIndex)
{
    return g_Script.pFuncTable[iIndex];
}

/* Prints an operand's of indirection */
void PrintOpIndir(int iOpIndex)
{
    int iIndirMethod = GetOpType(iOpIndex);
    switch (iIndirMethod) {
        case OP_TYPE_REG:
            printf("_RetVal");
            break;

        case OP_TYPE_ABS_STACK_INDEX:
        case OP_TYPE_REL_STACK_INDEX:
        {
            int iStackIndex = ResolveOpStackIndex(iOpIndex);
            printf("[%d]", iStackIndex);
            break;
        }
    }
}

void PrintOpValue(int iOpIndex)
{
    Value Op = ResolveOpValue(iOpIndex);
    switch (Op.iType)
    {
        case OP_TYPE_NULL:
            printf ("Null");
            break;

        case OP_TYPE_INT:
            printf ("%d", Op.iIntLiteral);
            break;

        case OP_TYPE_FLOAT:
            printf ("%f", Op.fFloatLiteral);
            break;

        case OP_TYPE_STRING:
            printf ("%s", Op.pstrStringLiteral);
            break;

        case OP_TYPE_INSTR_INDEX:
            printf ("%d", Op.iInstrIndex);
            break;

        case OP_TYPE_HOST_API_CALL_INDEX:
        {
            char * pstrHostAPICall = ResolveOpAsHostAPICall(iOpIndex);
            printf ("%s", pstrHostAPICall);
            break;
        }
    }
}

/* Returns the host API call corresponding to the specified index. */
char * GetHostAPICall(int iIndex)
{
    return g_Script.HostAPICallTable.ppstrCalls[iIndex];
}

int GetCurrTime()
{
    return (int) time(0);

}

int main()
{
}
