#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

// #define PURIFY

#define NEW(p, a) ((p) = allocate(sizeof *(p), (a)))
#define NEW0(p, a) memset(NEW((p), (a)), 0, sizeof *(p))
#define NELEMS(a) ((int)(sizeof(a) / sizeof((a)[0])))
#undef roundup
#define roundup(x, n) (((x) + ((n)-1)) & (~((n)-1)))

typedef struct symbol *Symbol;

typedef struct coord
{
	char *file;
	unsigned x, y; // y: line number, x: character position
} Coordinate;

typedef struct table *Table;

struct symbol
{
	char *name;		// identifier, if no name, generate number string
	int scope;		// constants, labels, global, param, local. E.g. local declaration in Level k, has scope local + k.
	Coordinate src; // pinpoint the symbol
	Symbol up;		// symbol list
	List uses;		// track for symbols
	int sclass;		// extend storage class. E.g. auto, register, static, extern.
	Type type;		// type information
	float ref;		// referenced times. see 10.3
	union {

	} u; // additional information
		 // Xsymbol x;
};

enum
{
	/* alloc blocks */
	PERM = 0, // permanent data
	FUNC,
	STMT
};

enum
{
	CONSTANTS = 1,
	LABELS,
	GLOBAL,
	PARAM,
	LOCAL
};

extern Table constants;
extern Table externals;
extern Table globals;
extern Table identifiers;
extern Table labels;
extern Table types;

extern void *allocate(unsigned long n, unsigned a);
extern void deallocate(unsigned a);
extern void *newarray(unsigned long m, unsigned long n, unsigned a);
extern char *string(const char *str);
extern char *stringn(const char *str, int len);
extern char *stringd(long n);