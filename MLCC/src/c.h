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

#define MAXLINE 512
#define BUFSIZE 4096

typedef struct symbol *Symbol;

typedef struct coord
{
	char *file;
	unsigned x, y; // y: line number, x: character position
} Coordinate;

typedef struct table *Table;

typedef union value {
	long i;
	unsigned long u;
	long double d;
	void *p;
	void (*g)(void);
} Value;

struct symbol
{
	char *name;		// identifier, if no name, generate number string
	int scope;		// constants, labels, global, param, local. E.g. local declaration in Level k, has scope local + k.
	Coordinate src; // pinpoint the symbol
	Symbol up;		// symbol list
	// List uses;		// track for symbols
	int sclass; // extend storage class. E.g. auto, register, static, extern.

	unsigned structarg : 1;
	unsigned addressed : 1;
	unsigned computed : 1;
	unsigned temporary : 1;
	unsigned generated : 1;
	unsigned defined : 1;

	// Type type;		// type information
	float ref; // referenced times. see 10.3
	union {
		/* label */
		struct
		{
			int label;
			Symbol equatedto;
		} l;

		/* const */
		struct
		{
			Value v;
			Symbol loc;
		} c;
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

enum
{
#define xx(a, b, c, d, e, f, g) a = b,
#define yy(a, b, c, d, e, f, g)
#include "token.h"
	LAST
};

extern Table constants;
extern Table externals;
extern Table globals;
extern Table identifiers;
extern Table labels;
extern Table types;
extern Coordinate src;
extern int level;
extern unsigned char *cp;
extern unsigned char *limit;
extern char *firstfile;
extern char *file;
extern char *line;
extern int lineno;

extern void *allocate(unsigned long n, unsigned a);
extern void deallocate(unsigned a);
extern void *newarray(unsigned long m, unsigned long n, unsigned a);
extern char *string(const char *str);
extern char *stringn(const char *str, int len);
extern char *stringd(long n);