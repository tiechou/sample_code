#include <stdlib.h>

void f(void)
{
    // problem 1: heap block overrun
    double* p1 = (double *)malloc(10 * sizeof(double));
    double* p2 = (double *)malloc(10 * sizeof(double));
    p1 = p2;
    free(p1);
    free(p2);
    
}                    

int main(void)
{
    f();
    return 0;
}
