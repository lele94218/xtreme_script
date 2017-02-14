//
//  main.h
//  assembler
//
//  Created by Taoran Xue on 1/16/17.
//  Copyright © 2017 Taoran Xue. All rights reserved.
//

#ifndef main_h
#define main_h


#endif /* main_h */

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

#define TOKEN_TYPE_INSTR      12      // An instruction

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

#define MAIN_FUNC_NAME        "_MAIN"   // _Main ()'s name

// ---- Error Strings ---------------------------------------------------------------------

// The following macros are used to represent assembly-time error strings

#define ERROR_MSSG_INVALID_INPUT  \
    "Invalid input"

#define ERROR_MSSG_INVALID_FILE     \
    "Could not open executable file for output"

#define ERROR_MSSG_LOCAL_SETSTACKSIZE \
    "SetStackSize can only appear in the global scope"

#define ERROR_MSSG_INVALID_STACK_SIZE \
    "Invalid stack size"

#define ERROR_MSSG_MULTIPLE_SETSTACKSIZES \
    "Multiple instances of SetStackSize illegal"

#define ERROR_MSSG_IDENT_EXPECTED \
    "Identifier expected"

#define ERROR_MSSG_INVALID_ARRAY_SIZE \
    "Invalid array size"

#define ERROR_MSSG_IDENT_REDEFINITION \
    "Identifier redefinition"

#define ERROR_MSSG_UNDEFINED_IDENT  \
    "Undefined identifier"

#define ERROR_MSSG_NESTED_FUNC  \
    "Nested functions illegal"

#define ERROR_MSSG_FUNC_REDEFINITION  \
    "Function redefinition"

#define ERROR_MSSG_UNDEFINED_FUNC \
    "Undefined function"

#define ERROR_MSSG_GLOBAL_PARAM \
    "Parameters can only appear inside functions"

#define ERROR_MSSG_MAIN_PARAM \
    "_Main () function cannot accept parameters"

#define ERROR_MSSG_GLOBAL_LINE_LABEL  \
    "Line labels can only appear inside functions"

#define ERROR_MSSG_LINE_LABEL_REDEFINITION  \
    "Line label redefinition"

#define ERROR_MSSG_UNDEFINED_LINE_LABEL \
    "Undefined line label"

#define ERROR_MSSG_GLOBAL_INSTR \
    "Instructions can only appear inside functions"

#define ERROR_MSSG_INVALID_INSTR  \
    "Invalid instruction"

#define ERROR_MSSG_INVALID_OP \
    "Invalid operand"

#define ERROR_MSSG_INVALID_STRING \
    "Invalid string"

#define ERROR_MSSG_INVALID_ARRAY_NOT_INDEXED  \
    "Arrays must be indexed"

#define ERROR_MSSG_INVALID_ARRAY  \
    "Invalid array"

#define ERROR_MSSG_INVALID_ARRAY_INDEX  \
    "Invalid array index"



// ---- Data Structures ----

typedef struct _Op
{
    int iType;                      // Type
    union
    {
        int iIntLiteral;        // Integer literal
        float fFloatLiteral;     // Float literal
        int iStringTableIndex;     // String table index
        int iStackIndex;        // Stack index
        int iInstrIndex;        // Instruction index
        int iFuncIndex;         // Function index
        int iHostAPICallIndex;     // Host API call index
        int iReg;               // Register code
    };
    int iOffsetIndex;               // Index of the offset
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
    int iCurrSourceLine;                            // Current line in the source

    unsigned int iIndex0,                           // Indices into the string
                 iIndex1;

    Token CurrToken;                                // Current token
    char pstrCurrLexeme [ MAX_LEXEME_SIZE ];        // Current lexeme

    int iCurrLexState;                              // The current lex state
}
Lexer;

// ---- Global Variables ----
Lexer g_Lexer;

// Source code representation
char ** g_ppstrSourceCode = NULL;
int g_iSourceCodeSize;

char g_pstrSourceFilename[MAX_FILENAME_SIZE], g_pstrExecFilename[MAX_FILENAME_SIZE];

// The instruction lookup table
InstrLookup g_InstrTable [ MAX_INSTR_LOOKUP_COUNT ];

// The assembled instruction stream
Instr * g_pInstrStream = NULL;                      // Pointer to a dynamically allocated instruction stream
int g_iInstrStreamSize;                             // The number of instructions
int g_iCurrInstrIndex;                              // The current instruction's index

// The script header
ScriptHeader g_ScriptHeader;
int g_iIsSetStackSizeFound;

FILE * g_pSourceFile = NULL;

// The main tables
LinkedList g_StringTable;
LinkedList g_FuncTable;
LinkedList g_SymbolTable;
LinkedList g_LabelTable;
LinkedList g_HostAPICallTable;

// ---- Function declarations ----
void ShutDown();

int AddString ( LinkedList * pList, char * pstrString );
int AddFunc ( char * pstrName, int iEntryPoint );
FuncNode * GetFuncByName( char * pstrName );
void SetFuncInfo ( char * pstrName, int iParamCount, int iLocalDataSize );

int AddSymbol ( char * pstrIdent, int iSize, int iStackIndex, int iFuncIndex );
SymbolNode * GetSymbolByIdent ( char * pstrIdent, int iFuncIndex );
int GetStackIndexByIdent ( char * pstrIdent, int iFuncIndex );
int GetSizeByIdent ( char * pstrIdent, int iFuncIndex );

int IsCharWhitespace (char cChar);

int AddLabel ( char * pstrIdent, int iTargetIndex, int iFuncIndex );
LabelNode * GetLabelByIdent ( char * pstrIdent, int iFuncIndex );

int AddInstrLookup ( char * pstrMnemonic, int iOpcode, int iOpCount );
void SetOpType ( int iInstrIndex, int iOpIndex, OpTypes iOpType );
int GetInstrByMnemonic ( char * pstrMnemonic, InstrLookup * pInstr );

int SkipToNextLine();
void ResetLexer ();
Token GetNextToken();
char GetLookAheadChar ();
char * GetCurrLexeme ();

void LoadSourceFile();
void BuildXSE();

void strupr(char * pstrString);
void Exit ();
void ExitOnError (char * pstrErrorMssg);
void ExitOnCodeError (const char * pstrErrorMssg);
void ExitOnCharExpectedError (char cChar);

void InitInstrTable ();
void PrintLogo ();
void PrintUsage ();
void Init ();
void PrintAssmblStats ();
