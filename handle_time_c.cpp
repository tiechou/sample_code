#include <stdio.h>  
#include <time.h>  

int main()  
{  
    struct tm tm_time;  
    strptime("2010-01-12 10:39:30", "%Y-%m-%d %H:%M:%S", &tm_time);  
    printf("%ld\n", mktime(&tm_time));  

    char szBuf[256] = {0};  
    time_t timer = time(NULL);  
    strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S", localtime(&timer));  
    printf("%s\n", szBuf);  

    return 0;  
} 
