#include "c.h"

struct str
{
    int a;
    int b;
    int c;
};

int main(int argc, char * argv[])
{  
    struct str a = {100};
    printf("%d %d %d\n", a.a, a.b, a.c);
}