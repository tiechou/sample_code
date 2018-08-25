#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<deque>
#include<sys/time.h>
using namespace std;
timeval start,end;
double timeuse;
void time()
{
    gettimeofday( &end, NULL );
    timeuse = (1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -     start.tv_usec)/1000000;
    printf("time used: %f s\n", timeuse);
}
int main(int argc, char** argv)
{
    gettimeofday(&start,NULL);
    for(int i=0;i<5000;i++)
    {
        //system("ls -l >/dev/null 2>&1 ");
        //popen( "ls -l", "r" );
        popen( "ls -l >/dev/null 2>&1 ", "r" );
    }
    time();
    return 0;
}

