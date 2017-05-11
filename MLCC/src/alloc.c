#include "c.h"

struct block
{
	struct block *next;
	char *limit;
	char *avail;
};

union align {
	long l;
	char *p;
	double d;
	int (*f)(void);
};

union header {
	struct block b;
	union align a;
};

struct tst_b
{
	char a;
	char b;
	char c;
	int d;
	char e;
};

union tst_h
{
	struct tst_b tb;
	union align a;
};
#ifdef PURIFY
union header *arena[3];

void *allocate(unsigned long n, unsigned a)
{
	union header *new = malloc(sizeof *new + n);

	/* test code begin */
	printf("%lu %lu\n", sizeof(union tst_h), sizeof(struct tst_b));
	struct tst_b tb;
	union tst_h th;
	printf("%p %p %p %p %p\n", &tb.a, &tb.b, &tb.c, &tb.d, &tb.e);
	printf("%p %p %p %p %p\n", &th.tb.a, &th.tb.b, &th.tb.c, &th.tb.d, &th.tb.e);
	/* test code end */



	assert(a < NELEMS(arena));
	if (new == NULL)
	{
		printf("insufficient memory\n");
		exit(1);
	}
	new->b.next = (void *)arena[a];
	arena[a] = new;
	/* move a sizeof struct block, to a new address for storing. */
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
static struct block
	first[] = {{NULL}, {NULL}, {NULL}}, // list head of arena 
	*arena[] = {&first[0], &first[1], &first[2]};

static struct block *freeblocks;

void *allocate(unsigned long n, unsigned a)
{
	struct block *ap;

	assert(a < NELEMS(arena));
	assert(n > 0);
	ap = arena[a];
	/* no space, reuse space in the free blocks. */
	while (ap->limit - ap->avail < n)
	{
		if ((ap->next = freeblocks) != NULL)
		{
			ap = ap->next;
			freeblocks = freeblocks->next;
		}
		else
		{
			unsigned int m = sizeof(union header) + n + roundup(10 * 1024, sizeof(union align));
			ap->next = malloc(m);
			ap = ap->next;
			if (ap == NULL)
			{
				printf("insufficient memory\n");
				exit(1);
			}
			/* must cast char * to add m bytes, otherwise will add m * sizeof(struct block). */
			ap->limit = (char *)ap + m; 
		}
		/* allocate must return a pointer suitably aligned to hold values of any type. */
		ap->avail = (char *)((union header *)ap + 1);
		ap->next = NULL;
		arena[a] = ap;
	}

	/* has space */
	ap->avail += n;
	return ap->avail - n;
}

void deallocate(unsigned a)
{
	assert(a < NELEMS(arena[a]));
	arena[a]->next = freeblocks;
	freeblocks = first[a].next;
	first[a].next = NULL;
	arena[a] = &first[a];
}

void *newarray(unsigned long m, unsigned long n, unsigned a)
{
	return allocate(m * n, a);
}
#endif