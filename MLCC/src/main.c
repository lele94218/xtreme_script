#include "c.h"


int main(int argc, char * argv[])
{  
    char * s = string("abc");
    char * t = string("abc");
    printf("%p %p\n", s, t);
}