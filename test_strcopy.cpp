#include <time.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
using namespace std;
const int SEC = 20000000;

int main()
{
    char buf[300];
    clock_t start = clock();
    for(int i = 0; i < SEC; ++i)
    {
        strcpy(buf,"woshixieyangbeijie");
    }

    clock_t finish = clock();
    double totaltime = (double)(finish - start)/CLOCKS_PER_SEC;
    fprintf(stderr,"strcpy time : %f s\n", totaltime);


    //strncpy
    start = clock();
    for(int i = 0; i < SEC; ++i)
    {
        int len = strlen("woshixieyangbeijie");
        strncpy(buf,"woshixieyangbeijie",len);
    }

    finish = clock();
    totaltime = (double)(finish - start)/CLOCKS_PER_SEC;
    fprintf(stderr,"strncpy time : %f s\n", totaltime);

    //memcpy
    start = clock();
    for(int i = 0; i < SEC; ++i)
    {
        int len = strlen("woshixieyangbeijie"); 
        memcpy(buf,"woshixieyangbeijie",len);
    }
    finish = clock();

    totaltime = (double)(finish - start)/CLOCKS_PER_SEC;
    fprintf(stderr,"memcpy time : %f s\n", totaltime);

    //snprintf
    start = clock();
    for(int i = 0; i < SEC; ++i)
    {
        snprintf(buf,300,"%s","woshixieyangbeijie");
    }

    finish = clock();
    totaltime = (double)(finish - start)/CLOCKS_PER_SEC;
    fprintf(stderr,"snprinf time : %f s\n", totaltime);

    return 0;

}

