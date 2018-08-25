#include <stdio.h>

int main()
{
    char *p = "const";
    char *q = NULL;
    if ( p = q ) {
        printf("%s\n", p);
    }
    return 0;
}
