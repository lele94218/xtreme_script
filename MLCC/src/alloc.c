#include "c.h"
struct block
{
    struct block * next;
    char * limit;
    char * avail;
};

union align
{
    long l;
    char * p;
    double d;
    int (*f)(void);
};

#ifdef PURIFY
union header *arena[3];

void *allocate(unsigned long n, unsigned a)
{
	union header *new = malloc(sizeof *new + n);

	assert(a < NELEMS(arena));
	if (new == NULL)
	{
		error("insufficient memory\n");
		exit(1);
	}
	new->b.next = (void *)arena[a];
	arena[a] = new;
	return new + 1;
}

void deallocate(unsigned a)
{
	union header *p, *q;

	assert(a < NELEMS(arena));
	for (p = arena[a]; p; p = q)
	{
		q = (void *)p->b.next;
		free(p);
	}
	arena[a] = NULL;
}

void *newarray(unsigned long m, unsigned long n, unsigned a)
{
	return allocate(m * n, a);
}
#else
#endif