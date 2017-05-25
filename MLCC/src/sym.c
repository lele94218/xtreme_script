#include "c.h"

struct table
{
    int level;
    Table previous;
    struct entry
    {
        struct symbol sym;
        struct entry *link; //list the hash entries of same level
    } * buckets[256];
    Symbol all;
};
#define HASHSIZE NELEMS(((Table)0)->buckets)

static struct table
    cns = {CONSTANTS},
    ext = {GLOBAL},
    ids = {GLOBAL},
    tys = {GLOBAL};

Table constans = &cns;
Table externels = &ext;
Table identifiers = &ids;
Table globals = &ids;
Table types = &tys;
Table labels;
int level = GLOBAL;
static int tempid;

Table newtable(int arena)
{
    Table new;
    NEW0(new, arena);
    return new;
}

Table table(Table tp, int level)
{
    Table new = newtable(FUNC);
    new->previous = tp;
    new->level = level;
    if (tp)
    {
        /* new table's all pointer is same as the previous table's */
        new->all = tp->all;
    }
    return new;
}

void foreach (Table tp, int lev, void (*apply)(Symbol, void *), void *cl)
{
    /* find the table whose level is the scope level */
    while (tp && tp->level > lev)
        tp = tp->previous;
    if (tp && tp->level == lev)
    {
        Symbol p;
        Coordinate sav;
        sav = src;
        for (p = tp->all; p && p->scope == lev; p = p->up)
        {
            src = p->src;
            (*apply)(p, cl);
        }
        src = sav;
    }
}

void enterscope(void)
{
    if (++level == LOCAL)
        tempid = 0;
}

void exitscope(void)
{
    rmtypes(level);
    if (types->level == level)
        types = types->previous;

    if (identifiers->level == level)
    {
        /* warn if more than 127 identifiers */
        identifiers = identifiers->previous;
    }
    assert(level >= GLOBAL);
    --level;
}

Symbol install(const char *name, Table *tpp, int level, int arena)
{
    Table tp = *tpp;
    struct entry *p;
    unsigned h = (unsigned long)name & (HASHSIZE - 1);

    assert(level == 0 || level >= tp->level);
    /* if level is nested or is the initial level, create new table */
    if (level > 0 && tp->level < level)
        tp = *tpp = table(tp, level);
    /* add p to hash list */
    NEW0(p, arena);
    p->sym.name = (char *)name;
    p->sym.scope = level;
    p->sym.up = tp->all;
    tp->all = &p->sym;
    p->link = tp->buckets[h];
    tp->buckets[h] = p;
    return &p->sym;
}

Symbol lookup(const char *name, Table tp)
{
    struct entry *p;
    unsigned h = (unsigned long)name & (HASHSIZE - 1);
    assert(tp);
    do
    {
        for (p = tp->buckets[h]; p; p = p->link)
        {
            if (name == p->sym.name)
                return &p->sym;
        }
    } while ((tp = tp->previous) != NULL);
    return NULL;
}

int genlabel(int n)
{
    static int label = 1;
    label += n;
    return label - n;
}

Symbol findlabel(int lab)
{
    struct entry *p;
    unsigned h = lab & (HASHSIZE - 1);
    for (p = labels->buckets[h]; p; p = p->link)
    {
        if (lab == p->sym.u.l.label)
            return &p->sym;
    }
    NEW0(p, FUNC);
    p->sym.name = stringd(lab);
    p->sym.scope = LABELS;
    p->sym.up = labels->all;
    labels->all = &p->sym;
    p->link = labels->buckets[h];
	labels->buckets[h] = p;
	p->sym.generated = 1;
	p->sym.u.l.label = lab;
	(*IR->defsymbol)(&p->sym);
	return &p->sym;
}

Symbol constant(Type ty, Value v) {
	struct entry *p;
	unsigned h = v.u&(HASHSIZE-1);
	static union { int x; char endian; } little = { 1 };

	ty = unqual(ty);
	for (p = constants->buckets[h]; p; p = p->link)
		if (eqtype(ty, p->sym.type, 1))
			switch (ty->op) {
			case INT:      if (equalp(i)) return &p->sym; break;
			case UNSIGNED: if (equalp(u)) return &p->sym; break;
			case FLOAT:
				if (v.d == 0.0) {
					float z1 = v.d, z2 = p->sym.u.c.v.d;
					char *b1 = (char *)&z1, *b2 = (char *)&z2;
					if (z1 == z2
					&& (!little.endian && b1[0] == b2[0]
					||   little.endian && b1[sizeof (z1)-1] == b2[sizeof (z2)-1]))
						return &p->sym;
				} else if (equalp(d))
					return &p->sym;
				break;
			case FUNCTION: if (equalp(g)) return &p->sym; break;
			case ARRAY:
			case POINTER:  if (equalp(p)) return &p->sym; break;
			default: assert(0);
			}
	NEW0(p, PERM);
	p->sym.name = vtoa(ty, v);
	p->sym.scope = CONSTANTS;
	p->sym.type = ty;
	p->sym.sclass = STATIC;
	p->sym.u.c.v = v;
	p->link = constants->buckets[h];
	p->sym.up = constants->all;
	constants->all = &p->sym;
	constants->buckets[h] = p;
	if (ty->u.sym && !ty->u.sym->addressed)
		(*IR->defsymbol)(&p->sym);
	p->sym.defined = 1;
	return &p->sym;
}