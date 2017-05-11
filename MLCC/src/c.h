#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#define PURIFY

#define NELEMS(a) ((int)(sizeof(a) / sizeof((a)[0])))
#undef roundup
#define roundup(x, n) (((x) + ((n) - 1)) & (~((n) - 1)))


extern void *allocate(unsigned long n, unsigned a);
extern void deallocate(unsigned a);
extern void *newarray(unsigned long m, unsigned long n, unsigned a);