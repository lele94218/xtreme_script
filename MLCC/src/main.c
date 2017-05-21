#include "c.h"

struct str
{
    int a;
    int b;
    int c;
};

void fun(int num)
{
    printf("%d\n", num);
}

int main(int argc, char * argv[])
{  
    void (*fun_pointer)(int);
    fun_pointer = fun;
    printf("%p %p %p\n", fun_pointer, *fun_pointer, &fun_pointer);
    printf("%p %p %p\n", fun, *fun, &fun);
    (*fun_pointer)(200);
    return 0;
}