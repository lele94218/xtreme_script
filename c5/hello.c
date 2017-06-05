#include <stdio.h>
#include <stdlib.h>
struct af {
    int p;
};
int main()
{
    struct af *a = NULL;
    struct af *b = a;
    b = malloc(sizeof(struct af));
    b->p = 100;
    printf("%d\n", a->p);
}