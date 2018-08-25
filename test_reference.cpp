#include<stdio.h> 

int f(int a,int *b=NULL){ 
    *b = 5;
}
int main()
{
    int b=0;
    f(2,&b); 
    printf("==%d==",b);
}
