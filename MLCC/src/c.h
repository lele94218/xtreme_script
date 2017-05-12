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
#define roundup(x, n) (((x) + ((n) - 1)) & (~((n) - 1)))


extern void *allocate(unsigned long n, unsigned a);
extern void deallocate(unsigned a);
extern void *newarray(unsigned long m, unsigned long n, unsigned a);


extern char *string(const char *str);
extern char *stringn(const char *str, int len);
extern char *stringd(long n);

enum
{
    /* alloc blocks */
	PERM = 0, // permanent data
	FUNC,
	STMT
};