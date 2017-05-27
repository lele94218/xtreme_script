#include "c.h"

int main(int argc, char * argv[])
{  
    assert(freopen("build/input.in", "r", stdin) != NULL);
    char string[100];
    int size = fread(&string[0], 1, 100000, stdin);
    string[size] = '\0';
    printf("%d: %s\n", size, string);
    return 0;
}