#include "c.h"

struct table
{
    int level;
    Table previous;
    struct entry
    {
        struct symbol sym;
        struct entry *link;
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
        new->all = tp->all;
    }
    return new;
}
