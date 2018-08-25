#include<stdio.h>
#include<stdint.h>

 void writeto(uint32_t data, uint32_t *encode,int width, int i)
 {
     int b = i * width;
     int e = b + width -1;
     int B = b >> 5;
     int E = e >> 5;
     int ls = b - ( B << 5 );
     encode[B] |= data << ls;
     if (E != B)
     {
         encode[E] |= data >> ( 32 - ls );
     }
 }

#include <string.h>

int main()
{
    uint32_t a[10] = {0};
    const char *b = "\x32\x34\x56\x78\x50";
    memcpy(a, b, strlen(b));
    writeto(5, a, 5, 7);

    //int b = 0xFFFF;
    return 0;
}
