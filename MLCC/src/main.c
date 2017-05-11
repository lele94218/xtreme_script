#include "c.h"

int main(int argc, char * argv[])
{
    int * a = newarray(10, sizeof(int), 1);
    printf("%lu\n", sizeof(char));
    printf("%p %p %p\n", a, a + 1, &a[1]);
    printf("hello\n");
}