#include <stdio.h>

enum st{
    yes = 48,
    no
};
int main()
{
    st t = yes;
    char c = '0';
    //char c = 48; 

    // here convent char to int 
    if (c==t){
        printf("==%d\n",t);
    }else {
        printf("!!%d , %d\n",c,t);
    }    
}
