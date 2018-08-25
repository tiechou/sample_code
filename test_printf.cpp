#include <stdio.h>
#include <stdlib.h>

int main()
{
    char str[128]="tiechouwanglifeng";
    printf("str=%.*s\n",2,str);
    char a[123] = "2323.1111";
    int at = strtol(a, NULL, 10);
    printf("str=%d\n", at);
}
