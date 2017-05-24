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
}