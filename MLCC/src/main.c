#include "c.h"


int main(int argc, char * argv[])
{
    int * a = newarray(10, sizeof(int), 1);
    printf("%lu %lu\n", sizeof(unsigned), sizeof(long));
    printf("%p %p %p\n", a, a + 1, &a[1]);
    printf("hello\n");
   
    
}