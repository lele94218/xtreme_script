#include <stdio.h>
#include <stdlib.h>

int main()
{
    int poolsize = 256 * 1024;
    int *p = malloc(poolsize);
    printf("%p\n", p);
    p = (int *)((int)p + poolsize);
    *(--p) = 10;
    *(--p) = 10;
    printf("done!\n");
}