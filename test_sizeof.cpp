#include<stdio.h>
#include<string.h>
#include<stdint.h>

#define tiechou 34733
typedef struct ib_text_s {
    uint64_t sign;
    uint32_t occ;
}ib_text_t;

int main()
{
    int a = tiechou;
    
    int count = 4;
    ib_text_t term[count];    
    int size = sizeof(term);
    fprintf(stderr, "size is %d\n", size);
    size = sizeof("src=taobao");
    fprintf(stderr, "size is %d\n", size);
    size = strlen("src=taobao");
    fprintf(stderr, "size is %d\n", size);
    return 0;
}
