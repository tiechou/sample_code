#include<iostream>
#if defined(__GNUC__) // GNU compiler
#define DLLEXPORT
#endif
extern "C" DLLEXPORT void hello()
{
    printf("hello taobao ");
}
int  main(int argc,char **argv)
{
    hello();
    return 0;
}
