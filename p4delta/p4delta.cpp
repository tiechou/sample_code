/******************************************************************
 *  Created on: 2013-4-16
 *      Author: yewang@taobao.com clm971910@gmail.com
 *
 *      Desc  : 提供 p4delta的  压缩和解压功能
 *
 ******************************************************************/

#include <algorithm>
#include <math.h>

#include "util/common.h"
#include "util/p4delta.h"
#include "util/varint.h"


namespace ups_util
{


/** 将docList转换为差值形式  */
inline static void
p4d_delta( uint32_t * arr, int num )
{
    int       align_num = num & 0xFFFFFFFC;
    uint32_t  curr      = 0;
    uint32_t  last      = 0;

    for ( int i = 0; i < align_num ; i +=4 )
    {
        uint32_t  arr0  = arr[ i ] ;
        uint32_t  arr1  = arr[ i + 1 ];
        uint32_t  arr2  = arr[ i + 2 ];
        uint32_t  arr3  = arr[ i + 3];

        arr[ i ]     = arr0 - last;
        arr[ i + 1 ] = arr1 - arr0;
        arr[ i + 2 ] = arr2 - arr1;
        arr[ i + 3 ] = arr3 - arr2;
        last         = arr3;
    }

    for ( int i = align_num; i < num; ++i )
    {
        curr     = arr[ i ];
        arr[ i ] = curr - last;
        last     = curr;
    }
}



//将差值形式还原
inline static void
p4d_deDelta( uint32_t * arr, int num )
{
    uint32_t  total     = 0;
    int       align_num = num & 0xFFFFFFFC;

    for ( int i = 0; i < align_num; i +=4 )
    {
        uint32_t  arr0  = arr[ i ];
        uint32_t  arr1  = arr[ i + 1 ];
        uint32_t  arr2  = arr[ i + 2 ];
        uint32_t  arr3  = arr[ i + 3];
        uint32_t  half1 = arr0 + arr1;
        uint32_t  half2 = arr2 + arr3;

        arr[ i ]     += total;
        arr[ i + 1 ] += total + arr0;
        arr[ i + 2 ] += total + half1;
        arr[ i + 3 ] += total + half1 + arr2;
        total        += half1 + half2;
    }

    if ( align_num > 0 )
    {
        for ( int i = align_num; i < num; ++i )
        {
            arr[ i ] += arr[ i - 1 ];
        }
    }
    else
    {
        for ( int i = 1; i < num; ++i )
        {
            arr[ i ] += arr[ i - 1 ];
        }
    }
}


// 计算 正常值 和 异常值 的边界 bit数，就是 在大于这个bit数的数值 就通通作为 异常值了
inline static int
p4d_getBitSize( uint32_t * arr, int num )
{
    int      bits       = 0;
    uint32_t value      = 0;
    int      offset     = (int)((num > 1) ? (num * BOUND_RATIO ) : 1);
    uint8_t  list[ 32 ] = {0};

    // 检测每种bit上 有多少个数字
    for ( int i = 0; i < num; ++i )
    {
        bits  = 0;
        value = arr[ i ];

        asm ("bsrl %1, %0;"
                :"=r"(bits)            /* output */
                :"r"(value) );         /* input */

        list[ bits ] ++;
    }

    bits = 0;
    while ( offset > 0 )
    {
        offset -= list[ bits++ ];
    }

    return bits;
}



// 向压缩存储区域写入一个data
inline static void
p4d_writeto( uint32_t data, uint32_t *encode, int width, int i )
{
    int b  = i * width;
    int e  = b + width -1;
    int B  = b >> 5;
    int E  = e >> 5;
    int ls = b - ( B << 5 );

    encode[B] |= data << ls;

    if (E != B)
    {
        encode[E] |= data >> ( 32 - ls );
    }
}



//解压缩到output中
static void
p4d_unpack_1( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        register uint32_t encode0 = encode[0];

        output[j+0]  = encode0         & 0x1;
        output[j+1]  = (encode0 >> 1)  & 0x1;
        output[j+2]  = (encode0 >> 2)  & 0x1;
        output[j+3]  = (encode0 >> 3)  & 0x1;
        output[j+4]  = (encode0 >> 4)  & 0x1;
        output[j+5]  = (encode0 >> 5)  & 0x1;
        output[j+6]  = (encode0 >> 6)  & 0x1;
        output[j+7]  = (encode0 >> 7)  & 0x1;
        output[j+8]  = (encode0 >> 8)  & 0x1;
        output[j+9]  = (encode0 >> 9)  & 0x1;
        output[j+10] = (encode0 >> 10) & 0x1;
        output[j+11] = (encode0 >> 11) & 0x1;
        output[j+12] = (encode0 >> 12) & 0x1;
        output[j+13] = (encode0 >> 13) & 0x1;
        output[j+14] = (encode0 >> 14) & 0x1;
        output[j+15] = (encode0 >> 15) & 0x1;
        output[j+16] = (encode0 >> 16) & 0x1;
        output[j+17] = (encode0 >> 17) & 0x1;
        output[j+18] = (encode0 >> 18) & 0x1;
        output[j+19] = (encode0 >> 19) & 0x1;
        output[j+20] = (encode0 >> 20) & 0x1;
        output[j+21] = (encode0 >> 21) & 0x1;
        output[j+22] = (encode0 >> 22) & 0x1;
        output[j+23] = (encode0 >> 23) & 0x1;
        output[j+24] = (encode0 >> 24) & 0x1;
        output[j+25] = (encode0 >> 25) & 0x1;
        output[j+26] = (encode0 >> 26) & 0x1;
        output[j+27] = (encode0 >> 27) & 0x1;
        output[j+28] = (encode0 >> 28) & 0x1;
        output[j+29] = (encode0 >> 29) & 0x1;
        output[j+30] = (encode0 >> 30) & 0x1;
        output[j+31] = (encode0 >> 31) & 0x1;

        encode += 1;
        j      += 32;
    }
}

static void
p4d_unpack_2( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        register uint32_t encode0 = encode[0];
        register uint32_t encode1 = encode[1];

        output[j+0]  = encode0 & 0x3;
        output[j+1]  = (encode0 >> 2) & 0x3;
        output[j+2]  = (encode0 >> 4) & 0x3;
        output[j+3]  = (encode0 >> 6) & 0x3;
        output[j+4]  = (encode0 >> 8) & 0x3;
        output[j+5]  = (encode0 >> 10) & 0x3;
        output[j+6]  = (encode0 >> 12) & 0x3;
        output[j+7]  = (encode0 >> 14) & 0x3;
        output[j+8]  = (encode0 >> 16) & 0x3;
        output[j+9]  = (encode0 >> 18) & 0x3;
        output[j+10] = (encode0 >> 20) & 0x3;
        output[j+11] = (encode0 >> 22) & 0x3;
        output[j+12] = (encode0 >> 24) & 0x3;
        output[j+13] = (encode0 >> 26) & 0x3;
        output[j+14] = (encode0 >> 28) & 0x3;
        output[j+15] = (encode0 >> 30) & 0x3;
        output[j+16] = encode1 & 0x3;
        output[j+17] = (encode1 >> 2) & 0x3;
        output[j+18] = (encode1 >> 4) & 0x3;
        output[j+19] = (encode1 >> 6) & 0x3;
        output[j+20] = (encode1 >> 8) & 0x3;
        output[j+21] = (encode1 >> 10) & 0x3;
        output[j+22] = (encode1 >> 12) & 0x3;
        output[j+23] = (encode1 >> 14) & 0x3;
        output[j+24] = (encode1 >> 16) & 0x3;
        output[j+25] = (encode1 >> 18) & 0x3;
        output[j+26] = (encode1 >> 20) & 0x3;
        output[j+27] = (encode1 >> 22) & 0x3;
        output[j+28] = (encode1 >> 24) & 0x3;
        output[j+29] = (encode1 >> 26) & 0x3;
        output[j+30] = (encode1 >> 28) & 0x3;
        output[j+31] = (encode1 >> 30) & 0x3;

        encode += 2;
        j      += 32;
    }
}


static void
p4d_unpack_3( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        register uint32_t encode0 = encode[0];
        register uint32_t encode1 = encode[1];
        register uint32_t encode2 = encode[2];

        output[j+0]  = encode0 & 0x7;
        output[j+1]  = (encode0 >> 3) & 0x7;
        output[j+2]  = (encode0 >> 6) & 0x7;
        output[j+3]  = (encode0 >> 9) & 0x7;
        output[j+4]  = (encode0 >> 12) & 0x7;
        output[j+5]  = (encode0 >> 15) & 0x7;
        output[j+6]  = (encode0 >> 18) & 0x7;
        output[j+7]  = (encode0 >> 21) & 0x7;
        output[j+8]  = (encode0 >> 24) & 0x7;
        output[j+9]  = (encode0 >> 27) & 0x7;
        output[j+10] = ((encode0 >> 30) | (encode1 << 2)) & 0x7;
        output[j+11] = (encode1 >> 1) & 0x7;
        output[j+12] = (encode1 >> 4) & 0x7;
        output[j+13] = (encode1 >> 7) & 0x7;
        output[j+14] = (encode1 >> 10) & 0x7;
        output[j+15] = (encode1 >> 13) & 0x7;
        output[j+16] = (encode1 >> 16) & 0x7;
        output[j+17] = (encode1 >> 19) & 0x7;
        output[j+18] = (encode1 >> 22) & 0x7;
        output[j+19] = (encode1 >> 25) & 0x7;
        output[j+20] = (encode1 >> 28) & 0x7;
        output[j+21] = ((encode1 >> 31) | (encode2 << 1)) & 0x7;
        output[j+22] = (encode2 >> 2) & 0x7;
        output[j+23] = (encode2 >> 5) & 0x7;
        output[j+24] = (encode2 >> 8) & 0x7;
        output[j+25] = (encode2 >> 11) & 0x7;
        output[j+26] = (encode2 >> 14) & 0x7;
        output[j+27] = (encode2 >> 17) & 0x7;
        output[j+28] = (encode2 >> 20) & 0x7;
        output[j+29] = (encode2 >> 23) & 0x7;
        output[j+30] = (encode2 >> 26) & 0x7;
        output[j+31] = (encode2 >> 29) & 0x7;

        encode += 3;
        j      += 32;
    }
}

static void
p4d_unpack_4( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        register uint32_t encode0 = encode[0];
        register uint32_t encode1 = encode[1];
        register uint32_t encode2 = encode[2];
        register uint32_t encode3 = encode[3];

        output[j+0]  = encode0 & 0xF;
        output[j+1]  = (encode0 >> 4) & 0xF;
        output[j+2]  = (encode0 >> 8) & 0xF;
        output[j+3]  = (encode0 >> 12) & 0xF;
        output[j+4]  = (encode0 >> 16) & 0xF;
        output[j+5]  = (encode0 >> 20) & 0xF;
        output[j+6]  = (encode0 >> 24) & 0xF;
        output[j+7]  = (encode0 >> 28) & 0xF;
        output[j+8]  = encode1 & 0xF;
        output[j+9]  = (encode1 >> 4) & 0xF;
        output[j+10] = (encode1 >> 8) & 0xF;
        output[j+11] = (encode1 >> 12) & 0xF;
        output[j+12] = (encode1 >> 16) & 0xF;
        output[j+13] = (encode1 >> 20) & 0xF;
        output[j+14] = (encode1 >> 24) & 0xF;
        output[j+15] = (encode1 >> 28) & 0xF;
        output[j+16] = encode2 & 0xF;
        output[j+17] = (encode2 >> 4) & 0xF;
        output[j+18] = (encode2 >> 8) & 0xF;
        output[j+19] = (encode2 >> 12) & 0xF;
        output[j+20] = (encode2 >> 16) & 0xF;
        output[j+21] = (encode2 >> 20) & 0xF;
        output[j+22] = (encode2 >> 24) & 0xF;
        output[j+23] = (encode2 >> 28) & 0xF;
        output[j+24] = encode3 & 0xF;
        output[j+25] = (encode3 >> 4) & 0xF;
        output[j+26] = (encode3 >> 8) & 0xF;
        output[j+27] = (encode3 >> 12) & 0xF;
        output[j+28] = (encode3 >> 16) & 0xF;
        output[j+29] = (encode3 >> 20) & 0xF;
        output[j+30] = (encode3 >> 24) & 0xF;
        output[j+31] = (encode3 >> 28) & 0xF;

        encode += 4;
        j      += 32;
    }
}

static void
p4d_unpack_5( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        register uint32_t encode0 = encode[0];
        register uint32_t encode1 = encode[1];
        register uint32_t encode2 = encode[2];
        register uint32_t encode3 = encode[3];
        register uint32_t encode4 = encode[4];

        output[j+0]  = encode0 & 0x1F;
        output[j+1]  = (encode0 >> 5) & 0x1F;
        output[j+2]  = (encode0 >> 10) & 0x1F;
        output[j+3]  = (encode0 >> 15) & 0x1F;
        output[j+4]  = (encode0 >> 20) & 0x1F;
        output[j+5]  = (encode0 >> 25) & 0x1F;
        output[j+6]  = ((encode0 >> 30) | (encode1 << 2)) & 0x1F;
        output[j+7]  = (encode1 >> 3) & 0x1F;
        output[j+8]  = (encode1 >> 8) & 0x1F;
        output[j+9]  = (encode1 >> 13) & 0x1F;
        output[j+10] = (encode1 >> 18) & 0x1F;
        output[j+11] = (encode1 >> 23) & 0x1F;
        output[j+12] = ((encode1 >> 28) | (encode2 << 4)) & 0x1F;
        output[j+13] = (encode2 >> 1) & 0x1F;
        output[j+14] = (encode2 >> 6) & 0x1F;
        output[j+15] = (encode2 >> 11) & 0x1F;
        output[j+16] = (encode2 >> 16) & 0x1F;
        output[j+17] = (encode2 >> 21) & 0x1F;
        output[j+18] = (encode2 >> 26) & 0x1F;
        output[j+19] = ((encode2 >> 31) | (encode3 << 1)) & 0x1F;
        output[j+20] = (encode3 >> 4) & 0x1F;
        output[j+21] = (encode3 >> 9) & 0x1F;
        output[j+22] = (encode3 >> 14) & 0x1F;
        output[j+23] = (encode3 >> 19) & 0x1F;
        output[j+24] = (encode3 >> 24) & 0x1F;
        output[j+25] = ((encode3 >> 29) | (encode4 << 3)) & 0x1F;
        output[j+26] = (encode4 >> 2) & 0x1F;
        output[j+27] = (encode4 >> 7) & 0x1F;
        output[j+28] = (encode4 >> 12) & 0x1F;
        output[j+29] = (encode4 >> 17) & 0x1F;
        output[j+30] = (encode4 >> 22) & 0x1F;
        output[j+31] = (encode4 >> 27) & 0x1F;

        encode += 5;
        j      += 32;
    }
}


static void
p4d_unpack_6( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        register uint32_t encode0 = encode[0];
        register uint32_t encode1 = encode[1];
        register uint32_t encode2 = encode[2];
        register uint32_t encode3 = encode[3];
        register uint32_t encode4 = encode[4];
        register uint32_t encode5 = encode[5];

        output[j+0]  = encode0 & 0x3F;
        output[j+1]  = (encode0 >> 6) & 0x3F;
        output[j+2]  = (encode0 >> 12) & 0x3F;
        output[j+3]  = (encode0 >> 18) & 0x3F;
        output[j+4]  = (encode0 >> 24) & 0x3F;
        output[j+5]  = ((encode0 >> 30) | (encode1 << 2)) & 0x3F;
        output[j+6]  = (encode1 >> 4) & 0x3F;
        output[j+7]  = (encode1 >> 10) & 0x3F;
        output[j+8]  = (encode1 >> 16) & 0x3F;
        output[j+9]  = (encode1 >> 22) & 0x3F;
        output[j+10] = ((encode1 >> 28) | (encode2 << 4)) & 0x3F;
        output[j+11] = (encode2 >> 2) & 0x3F;
        output[j+12] = (encode2 >> 8) & 0x3F;
        output[j+13] = (encode2 >> 14) & 0x3F;
        output[j+14] = (encode2 >> 20) & 0x3F;
        output[j+15] = (encode2 >> 26) & 0x3F;
        output[j+16] = encode3 & 0x3F;
        output[j+17] = (encode3 >> 6) & 0x3F;
        output[j+18] = (encode3 >> 12) & 0x3F;
        output[j+19] = (encode3 >> 18) & 0x3F;
        output[j+20] = (encode3 >> 24) & 0x3F;
        output[j+21] = ((encode3 >> 30) | (encode4 << 2)) & 0x3F;
        output[j+22] = (encode4 >> 4) & 0x3F;
        output[j+23] = (encode4 >> 10) & 0x3F;
        output[j+24] = (encode4 >> 16) & 0x3F;
        output[j+25] = (encode4 >> 22) & 0x3F;
        output[j+26] = ((encode4 >> 28) | (encode5 << 4)) & 0x3F;
        output[j+27] = (encode5 >> 2) & 0x3F;
        output[j+28] = (encode5 >> 8) & 0x3F;
        output[j+29] = (encode5 >> 14) & 0x3F;
        output[j+30] = (encode5 >> 20) & 0x3F;
        output[j+31] = (encode5 >> 26) & 0x3F;

        encode += 6;
        j      += 32;
    }
}

static void
p4d_unpack_7( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        register uint32_t encode0 = encode[0];
        register uint32_t encode1 = encode[1];
        register uint32_t encode2 = encode[2];
        register uint32_t encode3 = encode[3];
        register uint32_t encode4 = encode[4];
        register uint32_t encode5 = encode[5];
        register uint32_t encode6 = encode[6];

        output[j+0]  = encode0 & 0x7F;
        output[j+1]  = (encode0 >> 7) & 0x7F;
        output[j+2]  = (encode0 >> 14) & 0x7F;
        output[j+3]  = (encode0 >> 21) & 0x7F;
        output[j+4]  = ((encode0 >> 28) | (encode1 << 4)) & 0x7F;
        output[j+5]  = (encode1 >> 3) & 0x7F;
        output[j+6]  = (encode1 >> 10) & 0x7F;
        output[j+7]  = (encode1 >> 17) & 0x7F;
        output[j+8]  = (encode1 >> 24) & 0x7F;
        output[j+9]  = ((encode1 >> 31) | (encode2 << 1)) & 0x7F;
        output[j+10] = (encode2 >> 6) & 0x7F;
        output[j+11] = (encode2 >> 13) & 0x7F;
        output[j+12] = (encode2 >> 20) & 0x7F;
        output[j+13] = ((encode2 >> 27) | (encode3 << 5)) & 0x7F;
        output[j+14] = (encode3 >> 2) & 0x7F;
        output[j+15] = (encode3 >> 9) & 0x7F;
        output[j+16] = (encode3 >> 16) & 0x7F;
        output[j+17] = (encode3 >> 23) & 0x7F;
        output[j+18] = ((encode3 >> 30) | (encode4 << 2)) & 0x7F;
        output[j+19] = (encode4 >> 5) & 0x7F;
        output[j+20] = (encode4 >> 12) & 0x7F;
        output[j+21] = (encode4 >> 19) & 0x7F;
        output[j+22] = ((encode4 >> 26) | (encode5 << 6)) & 0x7F;
        output[j+23] = (encode5 >> 1) & 0x7F;
        output[j+24] = (encode5 >> 8) & 0x7F;
        output[j+25] = (encode5 >> 15) & 0x7F;
        output[j+26] = (encode5 >> 22) & 0x7F;
        output[j+27] = ((encode5 >> 29) | (encode6 << 3)) & 0x7F;
        output[j+28] = (encode6 >> 4) & 0x7F;
        output[j+29] = (encode6 >> 11) & 0x7F;
        output[j+30] = (encode6 >> 18) & 0x7F;
        output[j+31] = (encode6 >> 25) & 0x7F;

        encode += 7;
        j      += 32;
    }
}

static void
p4d_unpack_8( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        register uint32_t encode0 = encode[0];
        register uint32_t encode1 = encode[1];
        register uint32_t encode2 = encode[2];
        register uint32_t encode3 = encode[3];
        register uint32_t encode4 = encode[4];
        register uint32_t encode5 = encode[5];
        register uint32_t encode6 = encode[6];
        register uint32_t encode7 = encode[7];

        output[j+0]  = encode0 & 0xFF;
        output[j+1]  = (encode0 >> 8) & 0xFF;
        output[j+2]  = (encode0 >> 16) & 0xFF;
        output[j+3]  = (encode0 >> 24) & 0xFF;
        output[j+4]  = encode1 & 0xFF;
        output[j+5]  = (encode1 >> 8) & 0xFF;
        output[j+6]  = (encode1 >> 16) & 0xFF;
        output[j+7]  = (encode1 >> 24) & 0xFF;
        output[j+8]  = encode2 & 0xFF;
        output[j+9]  = (encode2 >> 8) & 0xFF;
        output[j+10] = (encode2 >> 16) & 0xFF;
        output[j+11] = (encode2 >> 24) & 0xFF;
        output[j+12] = encode3 & 0xFF;
        output[j+13] = (encode3 >> 8) & 0xFF;
        output[j+14] = (encode3 >> 16) & 0xFF;
        output[j+15] = (encode3 >> 24) & 0xFF;
        output[j+16] = encode4 & 0xFF;
        output[j+17] = (encode4 >> 8) & 0xFF;
        output[j+18] = (encode4 >> 16) & 0xFF;
        output[j+19] = (encode4 >> 24) & 0xFF;
        output[j+20] = encode5 & 0xFF;
        output[j+21] = (encode5 >> 8) & 0xFF;
        output[j+22] = (encode5 >> 16) & 0xFF;
        output[j+23] = (encode5 >> 24) & 0xFF;
        output[j+24] = encode6 & 0xFF;
        output[j+25] = (encode6 >> 8) & 0xFF;
        output[j+26] = (encode6 >> 16) & 0xFF;
        output[j+27] = (encode6 >> 24) & 0xFF;
        output[j+28] = encode7 & 0xFF;
        output[j+29] = (encode7 >> 8) & 0xFF;
        output[j+30] = (encode7 >> 16) & 0xFF;
        output[j+31] = (encode7 >> 24) & 0xFF;

        encode += 8;
        j      += 32;
    }
}

static void
p4d_unpack_9( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x1FF;
        output[j+1]  = (encode[0] >> 9) & 0x1FF;
        output[j+2]  = (encode[0] >> 18) & 0x1FF;
        output[j+3]  = ((encode[0] >> 27) | (encode[1] << 5)) & 0x1FF;
        output[j+4]  = (encode[1] >> 4) & 0x1FF;
        output[j+5]  = (encode[1] >> 13) & 0x1FF;
        output[j+6]  = (encode[1] >> 22) & 0x1FF;
        output[j+7]  = ((encode[1] >> 31) | (encode[2] << 1)) & 0x1FF;
        output[j+8]  = (encode[2] >> 8) & 0x1FF;
        output[j+9]  = (encode[2] >> 17) & 0x1FF;
        output[j+10] = ((encode[2] >> 26) | (encode[3] << 6)) & 0x1FF;
        output[j+11] = (encode[3] >> 3) & 0x1FF;
        output[j+12] = (encode[3] >> 12) & 0x1FF;
        output[j+13] = (encode[3] >> 21) & 0x1FF;
        output[j+14] = ((encode[3] >> 30) | (encode[4] << 2)) & 0x1FF;
        output[j+15] = (encode[4] >> 7) & 0x1FF;
        output[j+16] = (encode[4] >> 16) & 0x1FF;
        output[j+17] = ((encode[4] >> 25) | (encode[5] << 7)) & 0x1FF;
        output[j+18] = (encode[5] >> 2) & 0x1FF;
        output[j+19] = (encode[5] >> 11) & 0x1FF;
        output[j+20] = (encode[5] >> 20) & 0x1FF;
        output[j+21] = ((encode[5] >> 29) | (encode[6] << 3)) & 0x1FF;
        output[j+22] = (encode[6] >> 6) & 0x1FF;
        output[j+23] = (encode[6] >> 15) & 0x1FF;
        output[j+24] = ((encode[6] >> 24) | (encode[7] << 8)) & 0x1FF;
        output[j+25] = (encode[7] >> 1) & 0x1FF;
        output[j+26] = (encode[7] >> 10) & 0x1FF;
        output[j+27] = (encode[7] >> 19) & 0x1FF;
        output[j+28] = ((encode[7] >> 28) | (encode[8] << 4)) & 0x1FF;
        output[j+29] = (encode[8] >> 5) & 0x1FF;
        output[j+30] = (encode[8] >> 14) & 0x1FF;
        output[j+31] = (encode[8] >> 23) & 0x1FF;
        encode += 9;
        j += 32;
    }
}

static void
p4d_unpack_10( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x3FF;
        output[j+1]  = (encode[0] >> 10) & 0x3FF;
        output[j+2]  = (encode[0] >> 20) & 0x3FF;
        output[j+3]  = ((encode[0] >> 30) | (encode[1] << 2)) & 0x3FF;
        output[j+4]  = (encode[1] >> 8) & 0x3FF;
        output[j+5]  = (encode[1] >> 18) & 0x3FF;
        output[j+6]  = ((encode[1] >> 28) | (encode[2] << 4)) & 0x3FF;
        output[j+7]  = (encode[2] >> 6) & 0x3FF;
        output[j+8]  = (encode[2] >> 16) & 0x3FF;
        output[j+9]  = ((encode[2] >> 26) | (encode[3] << 6)) & 0x3FF;
        output[j+10] = (encode[3] >> 4) & 0x3FF;
        output[j+11] = (encode[3] >> 14) & 0x3FF;
        output[j+12] = ((encode[3] >> 24) | (encode[4] << 8)) & 0x3FF;
        output[j+13] = (encode[4] >> 2) & 0x3FF;
        output[j+14] = (encode[4] >> 12) & 0x3FF;
        output[j+15] = (encode[4] >> 22) & 0x3FF;
        output[j+16] = encode[5] & 0x3FF;
        output[j+17] = (encode[5] >> 10) & 0x3FF;
        output[j+18] = (encode[5] >> 20) & 0x3FF;
        output[j+19] = ((encode[5] >> 30) | (encode[6] << 2)) & 0x3FF;
        output[j+20] = (encode[6] >> 8) & 0x3FF;
        output[j+21] = (encode[6] >> 18) & 0x3FF;
        output[j+22] = ((encode[6] >> 28) | (encode[7] << 4)) & 0x3FF;
        output[j+23] = (encode[7] >> 6) & 0x3FF;
        output[j+24] = (encode[7] >> 16) & 0x3FF;
        output[j+25] = ((encode[7] >> 26) | (encode[8] << 6)) & 0x3FF;
        output[j+26] = (encode[8] >> 4) & 0x3FF;
        output[j+27] = (encode[8] >> 14) & 0x3FF;
        output[j+28] = ((encode[8] >> 24) | (encode[9] << 8)) & 0x3FF;
        output[j+29] = (encode[9] >> 2) & 0x3FF;
        output[j+30] = (encode[9] >> 12) & 0x3FF;
        output[j+31] = (encode[9] >> 22) & 0x3FF;
        encode += 10;
        j += 32;
    }
}


static void
p4d_unpack_11( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x7FF;
        output[j+1]  = (encode[0] >> 11) & 0x7FF;
        output[j+2]  = ((encode[0] >> 22) | (encode[1] << 10)) & 0x7FF;
        output[j+3]  = (encode[1] >> 1) & 0x7FF;
        output[j+4]  = (encode[1] >> 12) & 0x7FF;
        output[j+5]  = ((encode[1] >> 23) | (encode[2] << 9)) & 0x7FF;
        output[j+6]  = (encode[2] >> 2) & 0x7FF;
        output[j+7]  = (encode[2] >> 13) & 0x7FF;
        output[j+8]  = ((encode[2] >> 24) | (encode[3] << 8)) & 0x7FF;
        output[j+9]  = (encode[3] >> 3) & 0x7FF;
        output[j+10] = (encode[3] >> 14) & 0x7FF;
        output[j+11] = ((encode[3] >> 25) | (encode[4] << 7)) & 0x7FF;
        output[j+12] = (encode[4] >> 4) & 0x7FF;
        output[j+13] = (encode[4] >> 15) & 0x7FF;
        output[j+14] = ((encode[4] >> 26) | (encode[5] << 6)) & 0x7FF;
        output[j+15] = (encode[5] >> 5) & 0x7FF;
        output[j+16] = (encode[5] >> 16) & 0x7FF;
        output[j+17] = ((encode[5] >> 27) | (encode[6] << 5)) & 0x7FF;
        output[j+18] = (encode[6] >> 6) & 0x7FF;
        output[j+19] = (encode[6] >> 17) & 0x7FF;
        output[j+20] = ((encode[6] >> 28) | (encode[7] << 4)) & 0x7FF;
        output[j+21] = (encode[7] >> 7) & 0x7FF;
        output[j+22] = (encode[7] >> 18) & 0x7FF;
        output[j+23] = ((encode[7] >> 29) | (encode[8] << 3)) & 0x7FF;
        output[j+24] = (encode[8] >> 8) & 0x7FF;
        output[j+25] = (encode[8] >> 19) & 0x7FF;
        output[j+26] = ((encode[8] >> 30) | (encode[9] << 2)) & 0x7FF;
        output[j+27] = (encode[9] >> 9) & 0x7FF;
        output[j+28] = (encode[9] >> 20) & 0x7FF;
        output[j+29] = ((encode[9] >> 31) | (encode[10] << 1)) & 0x7FF;
        output[j+30] = (encode[10] >> 10) & 0x7FF;
        output[j+31] = (encode[10] >> 21) & 0x7FF;

        encode += 11;
        j      += 32;
    }
}

static void
p4d_unpack_12( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0xFFF;
        output[j+1]  = (encode[0] >> 12) & 0xFFF;
        output[j+2]  = ((encode[0] >> 24) | (encode[1] << 8)) & 0xFFF;
        output[j+3]  = (encode[1] >> 4) & 0xFFF;
        output[j+4]  = (encode[1] >> 16) & 0xFFF;
        output[j+5]  = ((encode[1] >> 28) | (encode[2] << 4)) & 0xFFF;
        output[j+6]  = (encode[2] >> 8) & 0xFFF;
        output[j+7]  = (encode[2] >> 20) & 0xFFF;
        output[j+8]  = encode[3] & 0xFFF;
        output[j+9]  = (encode[3] >> 12) & 0xFFF;
        output[j+10] = ((encode[3] >> 24) | (encode[4] << 8)) & 0xFFF;
        output[j+11] = (encode[4] >> 4) & 0xFFF;
        output[j+12] = (encode[4] >> 16) & 0xFFF;
        output[j+13] = ((encode[4] >> 28) | (encode[5] << 4)) & 0xFFF;
        output[j+14] = (encode[5] >> 8) & 0xFFF;
        output[j+15] = (encode[5] >> 20) & 0xFFF;
        output[j+16] = encode[6] & 0xFFF;
        output[j+17] = (encode[6] >> 12) & 0xFFF;
        output[j+18] = ((encode[6] >> 24) | (encode[7] << 8)) & 0xFFF;
        output[j+19] = (encode[7] >> 4) & 0xFFF;
        output[j+20] = (encode[7] >> 16) & 0xFFF;
        output[j+21] = ((encode[7] >> 28) | (encode[8] << 4)) & 0xFFF;
        output[j+22] = (encode[8] >> 8) & 0xFFF;
        output[j+23] = (encode[8] >> 20) & 0xFFF;
        output[j+24] = encode[9] & 0xFFF;
        output[j+25] = (encode[9] >> 12) & 0xFFF;
        output[j+26] = ((encode[9] >> 24) | (encode[10] << 8)) & 0xFFF;
        output[j+27] = (encode[10] >> 4) & 0xFFF;
        output[j+28] = (encode[10] >> 16) & 0xFFF;
        output[j+29] = ((encode[10] >> 28) | (encode[11] << 4)) & 0xFFF;
        output[j+30] = (encode[11] >> 8) & 0xFFF;
        output[j+31] = (encode[11] >> 20) & 0xFFF;

        encode += 12;
        j      += 32;
    }
}

static void
p4d_unpack_13( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x1FFF;
        output[j+1]  = (encode[0] >> 13) & 0x1FFF;
        output[j+2]  = ((encode[0] >> 26) | (encode[1] << 6)) & 0x1FFF;
        output[j+3]  = (encode[1] >> 7) & 0x1FFF;
        output[j+4]  = ((encode[1] >> 20) | (encode[2] << 12)) & 0x1FFF;
        output[j+5]  = (encode[2] >> 1) & 0x1FFF;
        output[j+6]  = (encode[2] >> 14) & 0x1FFF;
        output[j+7]  = ((encode[2] >> 27) | (encode[3] << 5)) & 0x1FFF;
        output[j+8]  = (encode[3] >> 8) & 0x1FFF;
        output[j+9]  = ((encode[3] >> 21) | (encode[4] << 11)) & 0x1FFF;
        output[j+10] = (encode[4] >> 2) & 0x1FFF;
        output[j+11] = (encode[4] >> 15) & 0x1FFF;
        output[j+12] = ((encode[4] >> 28) | (encode[5] << 4)) & 0x1FFF;
        output[j+13] = (encode[5] >> 9) & 0x1FFF;
        output[j+14] = ((encode[5] >> 22) | (encode[6] << 10)) & 0x1FFF;
        output[j+15] = (encode[6] >> 3) & 0x1FFF;
        output[j+16] = (encode[6] >> 16) & 0x1FFF;
        output[j+17] = ((encode[6] >> 29) | (encode[7] << 3)) & 0x1FFF;
        output[j+18] = (encode[7] >> 10) & 0x1FFF;
        output[j+19] = ((encode[7] >> 23) | (encode[8] << 9)) & 0x1FFF;
        output[j+20] = (encode[8] >> 4) & 0x1FFF;
        output[j+21] = (encode[8] >> 17) & 0x1FFF;
        output[j+22] = ((encode[8] >> 30) | (encode[9] << 2)) & 0x1FFF;
        output[j+23] = (encode[9] >> 11) & 0x1FFF;
        output[j+24] = ((encode[9] >> 24) | (encode[10] << 8)) & 0x1FFF;
        output[j+25] = (encode[10] >> 5) & 0x1FFF;
        output[j+26] = (encode[10] >> 18) & 0x1FFF;
        output[j+27] = ((encode[10] >> 31) | (encode[11] << 1)) & 0x1FFF;
        output[j+28] = (encode[11] >> 12) & 0x1FFF;
        output[j+29] = ((encode[11] >> 25) | (encode[12] << 7)) & 0x1FFF;
        output[j+30] = (encode[12] >> 6) & 0x1FFF;
        output[j+31] = (encode[12] >> 19) & 0x1FFF;

        encode += 13;
        j      += 32;
    }
}

static void
p4d_unpack_14( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x3FFF;
        output[j+1]  = (encode[0] >> 14) & 0x3FFF;
        output[j+2]  = ((encode[0] >> 28) | (encode[1] << 4)) & 0x3FFF;
        output[j+3]  = (encode[1] >> 10) & 0x3FFF;
        output[j+4]  = ((encode[1] >> 24) | (encode[2] << 8)) & 0x3FFF;
        output[j+5]  = (encode[2] >> 6) & 0x3FFF;
        output[j+6]  = ((encode[2] >> 20) | (encode[3] << 12)) & 0x3FFF;
        output[j+7]  = (encode[3] >> 2) & 0x3FFF;
        output[j+8]  = (encode[3] >> 16) & 0x3FFF;
        output[j+9]  = ((encode[3] >> 30) | (encode[4] << 2)) & 0x3FFF;
        output[j+10] = (encode[4] >> 12) & 0x3FFF;
        output[j+11] = ((encode[4] >> 26) | (encode[5] << 6)) & 0x3FFF;
        output[j+12] = (encode[5] >> 8) & 0x3FFF;
        output[j+13] = ((encode[5] >> 22) | (encode[6] << 10)) & 0x3FFF;
        output[j+14] = (encode[6] >> 4) & 0x3FFF;
        output[j+15] = (encode[6] >> 18) & 0x3FFF;
        output[j+16] = encode[7] & 0x3FFF;
        output[j+17] = (encode[7] >> 14) & 0x3FFF;
        output[j+18] = ((encode[7] >> 28) | (encode[8] << 4)) & 0x3FFF;
        output[j+19] = (encode[8] >> 10) & 0x3FFF;
        output[j+20] = ((encode[8] >> 24) | (encode[9] << 8)) & 0x3FFF;
        output[j+21] = (encode[9] >> 6) & 0x3FFF;
        output[j+22] = ((encode[9] >> 20) | (encode[10] << 12)) & 0x3FFF;
        output[j+23] = (encode[10] >> 2) & 0x3FFF;
        output[j+24] = (encode[10] >> 16) & 0x3FFF;
        output[j+25] = ((encode[10] >> 30) | (encode[11] << 2)) & 0x3FFF;
        output[j+26] = (encode[11] >> 12) & 0x3FFF;
        output[j+27] = ((encode[11] >> 26) | (encode[12] << 6)) & 0x3FFF;
        output[j+28] = (encode[12] >> 8) & 0x3FFF;
        output[j+29] = ((encode[12] >> 22) | (encode[13] << 10)) & 0x3FFF;
        output[j+30] = (encode[13] >> 4) & 0x3FFF;
        output[j+31] = (encode[13] >> 18) & 0x3FFF;

        encode += 14;
        j      += 32;
    }
}

static void
p4d_unpack_15( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x7FFF;
        output[j+1]  = (encode[0] >> 15) & 0x7FFF;
        output[j+2]  = ((encode[0] >> 30) | (encode[1] << 2)) & 0x7FFF;
        output[j+3]  = (encode[1] >> 13) & 0x7FFF;
        output[j+4]  = ((encode[1] >> 28) | (encode[2] << 4)) & 0x7FFF;
        output[j+5]  = (encode[2] >> 11) & 0x7FFF;
        output[j+6]  = ((encode[2] >> 26) | (encode[3] << 6)) & 0x7FFF;
        output[j+7]  = (encode[3] >> 9) & 0x7FFF;
        output[j+8]  = ((encode[3] >> 24) | (encode[4] << 8)) & 0x7FFF;
        output[j+9]  = (encode[4] >> 7) & 0x7FFF;
        output[j+10] = ((encode[4] >> 22) | (encode[5] << 10)) & 0x7FFF;
        output[j+11] = (encode[5] >> 5) & 0x7FFF;
        output[j+12] = ((encode[5] >> 20) | (encode[6] << 12)) & 0x7FFF;
        output[j+13] = (encode[6] >> 3) & 0x7FFF;
        output[j+14] = ((encode[6] >> 18) | (encode[7] << 14)) & 0x7FFF;
        output[j+15] = (encode[7] >> 1) & 0x7FFF;
        output[j+16] = (encode[7] >> 16) & 0x7FFF;
        output[j+17] = ((encode[7] >> 31) | (encode[8] << 1)) & 0x7FFF;
        output[j+18] = (encode[8] >> 14) & 0x7FFF;
        output[j+19] = ((encode[8] >> 29) | (encode[9] << 3)) & 0x7FFF;
        output[j+20] = (encode[9] >> 12) & 0x7FFF;
        output[j+21] = ((encode[9] >> 27) | (encode[10] << 5)) & 0x7FFF;
        output[j+22] = (encode[10] >> 10) & 0x7FFF;
        output[j+23] = ((encode[10] >> 25) | (encode[11] << 7)) & 0x7FFF;
        output[j+24] = (encode[11] >> 8) & 0x7FFF;
        output[j+25] = ((encode[11] >> 23) | (encode[12] << 9)) & 0x7FFF;
        output[j+26] = (encode[12] >> 6) & 0x7FFF;
        output[j+27] = ((encode[12] >> 21) | (encode[13] << 11)) & 0x7FFF;
        output[j+28] = (encode[13] >> 4) & 0x7FFF;
        output[j+29] = ((encode[13] >> 19) | (encode[14] << 13)) & 0x7FFF;
        output[j+30] = (encode[14] >> 2) & 0x7FFF;
        output[j+31] = (encode[14] >> 17) & 0x7FFF;

        encode += 15;
        j      += 32;
    }
}


static void
p4d_unpack_16( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0xFFFF;
        output[j+1]  = (encode[0] >> 16) & 0xFFFF;
        output[j+2]  = encode[1] & 0xFFFF;
        output[j+3]  = (encode[1] >> 16) & 0xFFFF;
        output[j+4]  = encode[2] & 0xFFFF;
        output[j+5]  = (encode[2] >> 16) & 0xFFFF;
        output[j+6]  = encode[3] & 0xFFFF;
        output[j+7]  = (encode[3] >> 16) & 0xFFFF;
        output[j+8]  = encode[4] & 0xFFFF;
        output[j+9]  = (encode[4] >> 16) & 0xFFFF;
        output[j+10] = encode[5] & 0xFFFF;
        output[j+11] = (encode[5] >> 16) & 0xFFFF;
        output[j+12] = encode[6] & 0xFFFF;
        output[j+13] = (encode[6] >> 16) & 0xFFFF;
        output[j+14] = encode[7] & 0xFFFF;
        output[j+15] = (encode[7] >> 16) & 0xFFFF;
        output[j+16] = encode[8] & 0xFFFF;
        output[j+17] = (encode[8] >> 16) & 0xFFFF;
        output[j+18] = encode[9] & 0xFFFF;
        output[j+19] = (encode[9] >> 16) & 0xFFFF;
        output[j+20] = encode[10] & 0xFFFF;
        output[j+21] = (encode[10] >> 16) & 0xFFFF;
        output[j+22] = encode[11] & 0xFFFF;
        output[j+23] = (encode[11] >> 16) & 0xFFFF;
        output[j+24] = encode[12] & 0xFFFF;
        output[j+25] = (encode[12] >> 16) & 0xFFFF;
        output[j+26] = encode[13] & 0xFFFF;
        output[j+27] = (encode[13] >> 16) & 0xFFFF;
        output[j+28] = encode[14] & 0xFFFF;
        output[j+29] = (encode[14] >> 16) & 0xFFFF;
        output[j+30] = encode[15] & 0xFFFF;
        output[j+31] = (encode[15] >> 16) & 0xFFFF;

        encode += 16;
        j      += 32;
    }

}

static void
p4d_unpack_17( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x1FFFF;
        output[j+1]  = ((encode[0] >> 17) | (encode[1] << 15)) & 0x1FFFF;
        output[j+2]  = (encode[1] >> 2) & 0x1FFFF;
        output[j+3]  = ((encode[1] >> 19) | (encode[2] << 13)) & 0x1FFFF;
        output[j+4]  = (encode[2] >> 4) & 0x1FFFF;
        output[j+5]  = ((encode[2] >> 21) | (encode[3] << 11)) & 0x1FFFF;
        output[j+6]  = (encode[3] >> 6) & 0x1FFFF;
        output[j+7]  = ((encode[3] >> 23) | (encode[4] << 9)) & 0x1FFFF;
        output[j+8]  = (encode[4] >> 8) & 0x1FFFF;
        output[j+9]  = ((encode[4] >> 25) | (encode[5] << 7)) & 0x1FFFF;
        output[j+10] = (encode[5] >> 10) & 0x1FFFF;
        output[j+11] = ((encode[5] >> 27) | (encode[6] << 5)) & 0x1FFFF;
        output[j+12] = (encode[6] >> 12) & 0x1FFFF;
        output[j+13] = ((encode[6] >> 29) | (encode[7] << 3)) & 0x1FFFF;
        output[j+14] = (encode[7] >> 14) & 0x1FFFF;
        output[j+15] = ((encode[7] >> 31) | (encode[8] << 1)) & 0x1FFFF;
        output[j+16] = ((encode[8] >> 16) | (encode[9] << 16)) & 0x1FFFF;
        output[j+17] = (encode[9] >> 1) & 0x1FFFF;
        output[j+18] = ((encode[9] >> 18) | (encode[10] << 14)) & 0x1FFFF;
        output[j+19] = (encode[10] >> 3) & 0x1FFFF;
        output[j+20] = ((encode[10] >> 20) | (encode[11] << 12)) & 0x1FFFF;
        output[j+21] = (encode[11] >> 5) & 0x1FFFF;
        output[j+22] = ((encode[11] >> 22) | (encode[12] << 10)) & 0x1FFFF;
        output[j+23] = (encode[12] >> 7) & 0x1FFFF;
        output[j+24] = ((encode[12] >> 24) | (encode[13] << 8)) & 0x1FFFF;
        output[j+25] = (encode[13] >> 9) & 0x1FFFF;
        output[j+26] = ((encode[13] >> 26) | (encode[14] << 6)) & 0x1FFFF;
        output[j+27] = (encode[14] >> 11) & 0x1FFFF;
        output[j+28] = ((encode[14] >> 28) | (encode[15] << 4)) & 0x1FFFF;
        output[j+29] = (encode[15] >> 13) & 0x1FFFF;
        output[j+30] = ((encode[15] >> 30) | (encode[16] << 2)) & 0x1FFFF;
        output[j+31] = (encode[16] >> 15) & 0x1FFFF;

        encode += 17;
        j      += 32;
    }
}

static void
p4d_unpack_18( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x3FFFF;
        output[j+1]  = ((encode[0] >> 18) | (encode[1] << 14)) & 0x3FFFF;
        output[j+2]  = (encode[1] >> 4) & 0x3FFFF;
        output[j+3]  = ((encode[1] >> 22) | (encode[2] << 10)) & 0x3FFFF;
        output[j+4]  = (encode[2] >> 8) & 0x3FFFF;
        output[j+5]  = ((encode[2] >> 26) | (encode[3] << 6)) & 0x3FFFF;
        output[j+6]  = (encode[3] >> 12) & 0x3FFFF;
        output[j+7]  = ((encode[3] >> 30) | (encode[4] << 2)) & 0x3FFFF;
        output[j+8]  = ((encode[4] >> 16) | (encode[5] << 16)) & 0x3FFFF;
        output[j+9]  = (encode[5] >> 2) & 0x3FFFF;
        output[j+10] = ((encode[5] >> 20) | (encode[6] << 12)) & 0x3FFFF;
        output[j+11] = (encode[6] >> 6) & 0x3FFFF;
        output[j+12] = ((encode[6] >> 24) | (encode[7] << 8)) & 0x3FFFF;
        output[j+13] = (encode[7] >> 10) & 0x3FFFF;
        output[j+14] = ((encode[7] >> 28) | (encode[8] << 4)) & 0x3FFFF;
        output[j+15] = (encode[8] >> 14) & 0x3FFFF;
        output[j+16] = encode[9] & 0x3FFFF;
        output[j+17] = ((encode[9] >> 18) | (encode[10] << 14)) & 0x3FFFF;
        output[j+18] = (encode[10] >> 4) & 0x3FFFF;
        output[j+19] = ((encode[10] >> 22) | (encode[11] << 10)) & 0x3FFFF;
        output[j+20] = (encode[11] >> 8) & 0x3FFFF;
        output[j+21] = ((encode[11] >> 26) | (encode[12] << 6)) & 0x3FFFF;
        output[j+22] = (encode[12] >> 12) & 0x3FFFF;
        output[j+23] = ((encode[12] >> 30) | (encode[13] << 2)) & 0x3FFFF;
        output[j+24] = ((encode[13] >> 16) | (encode[14] << 16)) & 0x3FFFF;
        output[j+25] = (encode[14] >> 2) & 0x3FFFF;
        output[j+26] = ((encode[14] >> 20) | (encode[15] << 12)) & 0x3FFFF;
        output[j+27] = (encode[15] >> 6) & 0x3FFFF;
        output[j+28] = ((encode[15] >> 24) | (encode[16] << 8)) & 0x3FFFF;
        output[j+29] = (encode[16] >> 10) & 0x3FFFF;
        output[j+30] = ((encode[16] >> 28) | (encode[17] << 4)) & 0x3FFFF;
        output[j+31] = (encode[17] >> 14) & 0x3FFFF;

        encode += 18;
        j      += 32;
    }
}

static void
p4d_unpack_19( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x7FFFF;
        output[j+1]  = ((encode[0] >> 19) | (encode[1] << 13)) & 0x7FFFF;
        output[j+2]  = (encode[1] >> 6) & 0x7FFFF;
        output[j+3]  = ((encode[1] >> 25) | (encode[2] << 7)) & 0x7FFFF;
        output[j+4]  = (encode[2] >> 12) & 0x7FFFF;
        output[j+5]  = ((encode[2] >> 31) | (encode[3] << 1)) & 0x7FFFF;
        output[j+6]  = ((encode[3] >> 18) | (encode[4] << 14)) & 0x7FFFF;
        output[j+7]  = (encode[4] >> 5) & 0x7FFFF;
        output[j+8]  = ((encode[4] >> 24) | (encode[5] << 8)) & 0x7FFFF;
        output[j+9]  = (encode[5] >> 11) & 0x7FFFF;
        output[j+10] = ((encode[5] >> 30) | (encode[6] << 2)) & 0x7FFFF;
        output[j+11] = ((encode[6] >> 17) | (encode[7] << 15)) & 0x7FFFF;
        output[j+12] = (encode[7] >> 4) & 0x7FFFF;
        output[j+13] = ((encode[7] >> 23) | (encode[8] << 9)) & 0x7FFFF;
        output[j+14] = (encode[8] >> 10) & 0x7FFFF;
        output[j+15] = ((encode[8] >> 29) | (encode[9] << 3)) & 0x7FFFF;
        output[j+16] = ((encode[9] >> 16) | (encode[10] << 16)) & 0x7FFFF;
        output[j+17] = (encode[10] >> 3) & 0x7FFFF;
        output[j+18] = ((encode[10] >> 22) | (encode[11] << 10)) & 0x7FFFF;
        output[j+19] = (encode[11] >> 9) & 0x7FFFF;
        output[j+20] = ((encode[11] >> 28) | (encode[12] << 4)) & 0x7FFFF;
        output[j+21] = ((encode[12] >> 15) | (encode[13] << 17)) & 0x7FFFF;
        output[j+22] = (encode[13] >> 2) & 0x7FFFF;
        output[j+23] = ((encode[13] >> 21) | (encode[14] << 11)) & 0x7FFFF;
        output[j+24] = (encode[14] >> 8) & 0x7FFFF;
        output[j+25] = ((encode[14] >> 27) | (encode[15] << 5)) & 0x7FFFF;
        output[j+26] = ((encode[15] >> 14) | (encode[16] << 18)) & 0x7FFFF;
        output[j+27] = (encode[16] >> 1) & 0x7FFFF;
        output[j+28] = ((encode[16] >> 20) | (encode[17] << 12)) & 0x7FFFF;
        output[j+29] = (encode[17] >> 7) & 0x7FFFF;
        output[j+30] = ((encode[17] >> 26) | (encode[18] << 6)) & 0x7FFFF;
        output[j+31] = (encode[18] >> 13) & 0x7FFFF;

        encode += 19;
        j      += 32;
    }
}

static void
p4d_unpack_20( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0xFFFFF;
        output[j+1]  = ((encode[0] >> 20) | (encode[1] << 12)) & 0xFFFFF;
        output[j+2]  = (encode[1] >> 8) & 0xFFFFF;
        output[j+3]  = ((encode[1] >> 28) | (encode[2] << 4)) & 0xFFFFF;
        output[j+4]  = ((encode[2] >> 16) | (encode[3] << 16)) & 0xFFFFF;
        output[j+5]  = (encode[3] >> 4) & 0xFFFFF;
        output[j+6]  = ((encode[3] >> 24) | (encode[4] << 8)) & 0xFFFFF;
        output[j+7]  = (encode[4] >> 12) & 0xFFFFF;
        output[j+8]  = encode[5] & 0xFFFFF;
        output[j+9]  = ((encode[5] >> 20) | (encode[6] << 12)) & 0xFFFFF;
        output[j+10] = (encode[6] >> 8) & 0xFFFFF;
        output[j+11] = ((encode[6] >> 28) | (encode[7] << 4)) & 0xFFFFF;
        output[j+12] = ((encode[7] >> 16) | (encode[8] << 16)) & 0xFFFFF;
        output[j+13] = (encode[8] >> 4) & 0xFFFFF;
        output[j+14] = ((encode[8] >> 24) | (encode[9] << 8)) & 0xFFFFF;
        output[j+15] = (encode[9] >> 12) & 0xFFFFF;
        output[j+16] = encode[10] & 0xFFFFF;
        output[j+17] = ((encode[10] >> 20) | (encode[11] << 12)) & 0xFFFFF;
        output[j+18] = (encode[11] >> 8) & 0xFFFFF;
        output[j+19] = ((encode[11] >> 28) | (encode[12] << 4)) & 0xFFFFF;
        output[j+20] = ((encode[12] >> 16) | (encode[13] << 16)) & 0xFFFFF;
        output[j+21] = (encode[13] >> 4) & 0xFFFFF;
        output[j+22] = ((encode[13] >> 24) | (encode[14] << 8)) & 0xFFFFF;
        output[j+23] = (encode[14] >> 12) & 0xFFFFF;
        output[j+24] = encode[15] & 0xFFFFF;
        output[j+25] = ((encode[15] >> 20) | (encode[16] << 12)) & 0xFFFFF;
        output[j+26] = (encode[16] >> 8) & 0xFFFFF;
        output[j+27] = ((encode[16] >> 28) | (encode[17] << 4)) & 0xFFFFF;
        output[j+28] = ((encode[17] >> 16) | (encode[18] << 16)) & 0xFFFFF;
        output[j+29] = (encode[18] >> 4) & 0xFFFFF;
        output[j+30] = ((encode[18] >> 24) | (encode[19] << 8)) & 0xFFFFF;
        output[j+31] = (encode[19] >> 12) & 0xFFFFF;

        encode += 20;
        j      += 32;
    }
}

static void
p4d_unpack_21( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x1FFFFF;
        output[j+1]  = ((encode[0] >> 21) | (encode[1] << 11)) & 0x1FFFFF;
        output[j+2]  = (encode[1] >> 10) & 0x1FFFFF;
        output[j+3]  = ((encode[1] >> 31) | (encode[2] << 1)) & 0x1FFFFF;
        output[j+4]  = ((encode[2] >> 20) | (encode[3] << 12)) & 0x1FFFFF;
        output[j+5]  = (encode[3] >> 9) & 0x1FFFFF;
        output[j+6]  = ((encode[3] >> 30) | (encode[4] << 2)) & 0x1FFFFF;
        output[j+7]  = ((encode[4] >> 19) | (encode[5] << 13)) & 0x1FFFFF;
        output[j+8]  = (encode[5] >> 8) & 0x1FFFFF;
        output[j+9]  = ((encode[5] >> 29) | (encode[6] << 3)) & 0x1FFFFF;
        output[j+10] = ((encode[6] >> 18) | (encode[7] << 14)) & 0x1FFFFF;
        output[j+11] = (encode[7] >> 7) & 0x1FFFFF;
        output[j+12] = ((encode[7] >> 28) | (encode[8] << 4)) & 0x1FFFFF;
        output[j+13] = ((encode[8] >> 17) | (encode[9] << 15)) & 0x1FFFFF;
        output[j+14] = (encode[9] >> 6) & 0x1FFFFF;
        output[j+15] = ((encode[9] >> 27) | (encode[10] << 5)) & 0x1FFFFF;
        output[j+16] = ((encode[10] >> 16) | (encode[11] << 16)) & 0x1FFFFF;
        output[j+17] = (encode[11] >> 5) & 0x1FFFFF;
        output[j+18] = ((encode[11] >> 26) | (encode[12] << 6)) & 0x1FFFFF;
        output[j+19] = ((encode[12] >> 15) | (encode[13] << 17)) & 0x1FFFFF;
        output[j+20] = (encode[13] >> 4) & 0x1FFFFF;
        output[j+21] = ((encode[13] >> 25) | (encode[14] << 7)) & 0x1FFFFF;
        output[j+22] = ((encode[14] >> 14) | (encode[15] << 18)) & 0x1FFFFF;
        output[j+23] = (encode[15] >> 3) & 0x1FFFFF;
        output[j+24] = ((encode[15] >> 24) | (encode[16] << 8)) & 0x1FFFFF;
        output[j+25] = ((encode[16] >> 13) | (encode[17] << 19)) & 0x1FFFFF;
        output[j+26] = (encode[17] >> 2) & 0x1FFFFF;
        output[j+27] = ((encode[17] >> 23) | (encode[18] << 9)) & 0x1FFFFF;
        output[j+28] = ((encode[18] >> 12) | (encode[19] << 20)) & 0x1FFFFF;
        output[j+29] = (encode[19] >> 1) & 0x1FFFFF;
        output[j+30] = ((encode[19] >> 22) | (encode[20] << 10)) & 0x1FFFFF;
        output[j+31] = (encode[20] >> 11) & 0x1FFFFF;

        encode += 21;
        j      += 32;
    }
}

static void
p4d_unpack_22( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x3FFFFF;
        output[j+1]  = ((encode[0] >> 22) | (encode[1] << 10)) & 0x3FFFFF;
        output[j+2]  = ((encode[1] >> 12) | (encode[2] << 20)) & 0x3FFFFF;
        output[j+3]  = (encode[2] >> 2) & 0x3FFFFF;
        output[j+4]  = ((encode[2] >> 24) | (encode[3] << 8)) & 0x3FFFFF;
        output[j+5]  = ((encode[3] >> 14) | (encode[4] << 18)) & 0x3FFFFF;
        output[j+6]  = (encode[4] >> 4) & 0x3FFFFF;
        output[j+7]  = ((encode[4] >> 26) | (encode[5] << 6)) & 0x3FFFFF;
        output[j+8]  = ((encode[5] >> 16) | (encode[6] << 16)) & 0x3FFFFF;
        output[j+9]  = (encode[6] >> 6) & 0x3FFFFF;
        output[j+10] = ((encode[6] >> 28) | (encode[7] << 4)) & 0x3FFFFF;
        output[j+11] = ((encode[7] >> 18) | (encode[8] << 14)) & 0x3FFFFF;
        output[j+12] = (encode[8] >> 8) & 0x3FFFFF;
        output[j+13] = ((encode[8] >> 30) | (encode[9] << 2)) & 0x3FFFFF;
        output[j+14] = ((encode[9] >> 20) | (encode[10] << 12)) & 0x3FFFFF;
        output[j+15] = (encode[10] >> 10) & 0x3FFFFF;
        output[j+16] = encode[11] & 0x3FFFFF;
        output[j+17] = ((encode[11] >> 22) | (encode[12] << 10)) & 0x3FFFFF;
        output[j+18] = ((encode[12] >> 12) | (encode[13] << 20)) & 0x3FFFFF;
        output[j+19] = (encode[13] >> 2) & 0x3FFFFF;
        output[j+20] = ((encode[13] >> 24) | (encode[14] << 8)) & 0x3FFFFF;
        output[j+21] = ((encode[14] >> 14) | (encode[15] << 18)) & 0x3FFFFF;
        output[j+22] = (encode[15] >> 4) & 0x3FFFFF;
        output[j+23] = ((encode[15] >> 26) | (encode[16] << 6)) & 0x3FFFFF;
        output[j+24] = ((encode[16] >> 16) | (encode[17] << 16)) & 0x3FFFFF;
        output[j+25] = (encode[17] >> 6) & 0x3FFFFF;
        output[j+26] = ((encode[17] >> 28) | (encode[18] << 4)) & 0x3FFFFF;
        output[j+27] = ((encode[18] >> 18) | (encode[19] << 14)) & 0x3FFFFF;
        output[j+28] = (encode[19] >> 8) & 0x3FFFFF;
        output[j+29] = ((encode[19] >> 30) | (encode[20] << 2)) & 0x3FFFFF;
        output[j+30] = ((encode[20] >> 20) | (encode[21] << 12)) & 0x3FFFFF;
        output[j+31] = (encode[21] >> 10) & 0x3FFFFF;

        encode += 22;
        j      += 32;
    }
}

static void
p4d_unpack_23( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x7FFFFF;
        output[j+1]  = ((encode[0] >> 23) | (encode[1] << 9)) & 0x7FFFFF;
        output[j+2]  = ((encode[1] >> 14) | (encode[2] << 18)) & 0x7FFFFF;
        output[j+3]  = (encode[2] >> 5) & 0x7FFFFF;
        output[j+4]  = ((encode[2] >> 28) | (encode[3] << 4)) & 0x7FFFFF;
        output[j+5]  = ((encode[3] >> 19) | (encode[4] << 13)) & 0x7FFFFF;
        output[j+6]  = ((encode[4] >> 10) | (encode[5] << 22)) & 0x7FFFFF;
        output[j+7]  = (encode[5] >> 1) & 0x7FFFFF;
        output[j+8]  = ((encode[5] >> 24) | (encode[6] << 8)) & 0x7FFFFF;
        output[j+9]  = ((encode[6] >> 15) | (encode[7] << 17)) & 0x7FFFFF;
        output[j+10] = (encode[7] >> 6) & 0x7FFFFF;
        output[j+11] = ((encode[7] >> 29) | (encode[8] << 3)) & 0x7FFFFF;
        output[j+12] = ((encode[8] >> 20) | (encode[9] << 12)) & 0x7FFFFF;
        output[j+13] = ((encode[9] >> 11) | (encode[10] << 21)) & 0x7FFFFF;
        output[j+14] = (encode[10] >> 2) & 0x7FFFFF;
        output[j+15] = ((encode[10] >> 25) | (encode[11] << 7)) & 0x7FFFFF;
        output[j+16] = ((encode[11] >> 16) | (encode[12] << 16)) & 0x7FFFFF;
        output[j+17] = (encode[12] >> 7) & 0x7FFFFF;
        output[j+18] = ((encode[12] >> 30) | (encode[13] << 2)) & 0x7FFFFF;
        output[j+19] = ((encode[13] >> 21) | (encode[14] << 11)) & 0x7FFFFF;
        output[j+20] = ((encode[14] >> 12) | (encode[15] << 20)) & 0x7FFFFF;
        output[j+21] = (encode[15] >> 3) & 0x7FFFFF;
        output[j+22] = ((encode[15] >> 26) | (encode[16] << 6)) & 0x7FFFFF;
        output[j+23] = ((encode[16] >> 17) | (encode[17] << 15)) & 0x7FFFFF;
        output[j+24] = (encode[17] >> 8) & 0x7FFFFF;
        output[j+25] = ((encode[17] >> 31) | (encode[18] << 1)) & 0x7FFFFF;
        output[j+26] = ((encode[18] >> 22) | (encode[19] << 10)) & 0x7FFFFF;
        output[j+27] = ((encode[19] >> 13) | (encode[20] << 19)) & 0x7FFFFF;
        output[j+28] = (encode[20] >> 4) & 0x7FFFFF;
        output[j+29] = ((encode[20] >> 27) | (encode[21] << 5)) & 0x7FFFFF;
        output[j+30] = ((encode[21] >> 18) | (encode[22] << 14)) & 0x7FFFFF;
        output[j+31] = (encode[22] >> 9) & 0x7FFFFF;

        encode += 23;
        j      += 32;
    }
}

static void
p4d_unpack_24( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0xFFFFFF;
        output[j+1]  = ((encode[0] >> 24) | (encode[1] << 8)) & 0xFFFFFF;
        output[j+2]  = ((encode[1] >> 16) | (encode[2] << 16)) & 0xFFFFFF;
        output[j+3]  = (encode[2] >> 8) & 0xFFFFFF;
        output[j+4]  = encode[3] & 0xFFFFFF;
        output[j+5]  = ((encode[3] >> 24) | (encode[4] << 8)) & 0xFFFFFF;
        output[j+6]  = ((encode[4] >> 16) | (encode[5] << 16)) & 0xFFFFFF;
        output[j+7]  = (encode[5] >> 8) & 0xFFFFFF;
        output[j+8]  = encode[6] & 0xFFFFFF;
        output[j+9]  = ((encode[6] >> 24) | (encode[7] << 8)) & 0xFFFFFF;
        output[j+10] = ((encode[7] >> 16) | (encode[8] << 16)) & 0xFFFFFF;
        output[j+11] = (encode[8] >> 8) & 0xFFFFFF;
        output[j+12] = encode[9] & 0xFFFFFF;
        output[j+13] = ((encode[9] >> 24) | (encode[10] << 8)) & 0xFFFFFF;
        output[j+14] = ((encode[10] >> 16) | (encode[11] << 16)) & 0xFFFFFF;
        output[j+15] = (encode[11] >> 8) & 0xFFFFFF;
        output[j+16] = encode[12] & 0xFFFFFF;
        output[j+17] = ((encode[12] >> 24) | (encode[13] << 8)) & 0xFFFFFF;
        output[j+18] = ((encode[13] >> 16) | (encode[14] << 16)) & 0xFFFFFF;
        output[j+19] = (encode[14] >> 8) & 0xFFFFFF;
        output[j+20] = encode[15] & 0xFFFFFF;
        output[j+21] = ((encode[15] >> 24) | (encode[16] << 8)) & 0xFFFFFF;
        output[j+22] = ((encode[16] >> 16) | (encode[17] << 16)) & 0xFFFFFF;
        output[j+23] = (encode[17] >> 8) & 0xFFFFFF;
        output[j+24] = encode[18] & 0xFFFFFF;
        output[j+25] = ((encode[18] >> 24) | (encode[19] << 8)) & 0xFFFFFF;
        output[j+26] = ((encode[19] >> 16) | (encode[20] << 16)) & 0xFFFFFF;
        output[j+27] = (encode[20] >> 8) & 0xFFFFFF;
        output[j+28] = encode[21] & 0xFFFFFF;
        output[j+29] = ((encode[21] >> 24) | (encode[22] << 8)) & 0xFFFFFF;
        output[j+30] = ((encode[22] >> 16) | (encode[23] << 16)) & 0xFFFFFF;
        output[j+31] = (encode[23] >> 8) & 0xFFFFFF;

        encode += 24;
        j      += 32;
    }
}

static void
p4d_unpack_25( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x1FFFFFF;
        output[j+1]  = ((encode[0] >> 25) | (encode[1] << 7)) & 0x1FFFFFF;
        output[j+2]  = ((encode[1] >> 18) | (encode[2] << 14)) & 0x1FFFFFF;
        output[j+3]  = ((encode[2] >> 11) | (encode[3] << 21)) & 0x1FFFFFF;
        output[j+4]  = (encode[3] >> 4) & 0x1FFFFFF;
        output[j+5]  = ((encode[3] >> 29) | (encode[4] << 3)) & 0x1FFFFFF;
        output[j+6]  = ((encode[4] >> 22) | (encode[5] << 10)) & 0x1FFFFFF;
        output[j+7]  = ((encode[5] >> 15) | (encode[6] << 17)) & 0x1FFFFFF;
        output[j+8]  = ((encode[6] >> 8) | (encode[7] << 24)) & 0x1FFFFFF;
        output[j+9]  = (encode[7] >> 1) & 0x1FFFFFF;
        output[j+10] = ((encode[7] >> 26) | (encode[8] << 6)) & 0x1FFFFFF;
        output[j+11] = ((encode[8] >> 19) | (encode[9] << 13)) & 0x1FFFFFF;
        output[j+12] = ((encode[9] >> 12) | (encode[10] << 20)) & 0x1FFFFFF;
        output[j+13] = (encode[10] >> 5) & 0x1FFFFFF;
        output[j+14] = ((encode[10] >> 30) | (encode[11] << 2)) & 0x1FFFFFF;
        output[j+15] = ((encode[11] >> 23) | (encode[12] << 9)) & 0x1FFFFFF;
        output[j+16] = ((encode[12] >> 16) | (encode[13] << 16)) & 0x1FFFFFF;
        output[j+17] = ((encode[13] >> 9) | (encode[14] << 23)) & 0x1FFFFFF;
        output[j+18] = (encode[14] >> 2) & 0x1FFFFFF;
        output[j+19] = ((encode[14] >> 27) | (encode[15] << 5)) & 0x1FFFFFF;
        output[j+20] = ((encode[15] >> 20) | (encode[16] << 12)) & 0x1FFFFFF;
        output[j+21] = ((encode[16] >> 13) | (encode[17] << 19)) & 0x1FFFFFF;
        output[j+22] = (encode[17] >> 6) & 0x1FFFFFF;
        output[j+23] = ((encode[17] >> 31) | (encode[18] << 1)) & 0x1FFFFFF;
        output[j+24] = ((encode[18] >> 24) | (encode[19] << 8)) & 0x1FFFFFF;
        output[j+25] = ((encode[19] >> 17) | (encode[20] << 15)) & 0x1FFFFFF;
        output[j+26] = ((encode[20] >> 10) | (encode[21] << 22)) & 0x1FFFFFF;
        output[j+27] = (encode[21] >> 3) & 0x1FFFFFF;
        output[j+28] = ((encode[21] >> 28) | (encode[22] << 4)) & 0x1FFFFFF;
        output[j+29] = ((encode[22] >> 21) | (encode[23] << 11)) & 0x1FFFFFF;
        output[j+30] = ((encode[23] >> 14) | (encode[24] << 18)) & 0x1FFFFFF;
        output[j+31] = (encode[24] >> 7) & 0x1FFFFFF;

        encode += 25;
        j      += 32;
    }
}

static void
p4d_unpack_26( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x3FFFFFF;
        output[j+1]  = ((encode[0] >> 26) | (encode[1] << 6)) & 0x3FFFFFF;
        output[j+2]  = ((encode[1] >> 20) | (encode[2] << 12)) & 0x3FFFFFF;
        output[j+3]  = ((encode[2] >> 14) | (encode[3] << 18)) & 0x3FFFFFF;
        output[j+4]  = ((encode[3] >> 8) | (encode[4] << 24)) & 0x3FFFFFF;
        output[j+5]  = (encode[4] >> 2) & 0x3FFFFFF;
        output[j+6]  = ((encode[4] >> 28) | (encode[5] << 4)) & 0x3FFFFFF;
        output[j+7]  = ((encode[5] >> 22) | (encode[6] << 10)) & 0x3FFFFFF;
        output[j+8]  = ((encode[6] >> 16) | (encode[7] << 16)) & 0x3FFFFFF;
        output[j+9]  = ((encode[7] >> 10) | (encode[8] << 22)) & 0x3FFFFFF;
        output[j+10] = (encode[8] >> 4) & 0x3FFFFFF;
        output[j+11] = ((encode[8] >> 30) | (encode[9] << 2)) & 0x3FFFFFF;
        output[j+12] = ((encode[9] >> 24) | (encode[10] << 8)) & 0x3FFFFFF;
        output[j+13] = ((encode[10] >> 18) | (encode[11] << 14)) & 0x3FFFFFF;
        output[j+14] = ((encode[11] >> 12) | (encode[12] << 20)) & 0x3FFFFFF;
        output[j+15] = (encode[12] >> 6) & 0x3FFFFFF;
        output[j+16] = encode[13] & 0x3FFFFFF;
        output[j+17] = ((encode[13] >> 26) | (encode[14] << 6)) & 0x3FFFFFF;
        output[j+18] = ((encode[14] >> 20) | (encode[15] << 12)) & 0x3FFFFFF;
        output[j+19] = ((encode[15] >> 14) | (encode[16] << 18)) & 0x3FFFFFF;
        output[j+20] = ((encode[16] >> 8) | (encode[17] << 24)) & 0x3FFFFFF;
        output[j+21] = (encode[17] >> 2) & 0x3FFFFFF;
        output[j+22] = ((encode[17] >> 28) | (encode[18] << 4)) & 0x3FFFFFF;
        output[j+23] = ((encode[18] >> 22) | (encode[19] << 10)) & 0x3FFFFFF;
        output[j+24] = ((encode[19] >> 16) | (encode[20] << 16)) & 0x3FFFFFF;
        output[j+25] = ((encode[20] >> 10) | (encode[21] << 22)) & 0x3FFFFFF;
        output[j+26] = (encode[21] >> 4) & 0x3FFFFFF;
        output[j+27] = ((encode[21] >> 30) | (encode[22] << 2)) & 0x3FFFFFF;
        output[j+28] = ((encode[22] >> 24) | (encode[23] << 8)) & 0x3FFFFFF;
        output[j+29] = ((encode[23] >> 18) | (encode[24] << 14)) & 0x3FFFFFF;
        output[j+30] = ((encode[24] >> 12) | (encode[25] << 20)) & 0x3FFFFFF;
        output[j+31] = (encode[25] >> 6) & 0x3FFFFFF;

        encode += 26;
        j      += 32;
    }
}

static void
p4d_unpack_27( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x7FFFFFF;
        output[j+1]  = ((encode[0] >> 27) | (encode[1] << 5)) & 0x7FFFFFF;
        output[j+2]  = ((encode[1] >> 22) | (encode[2] << 10)) & 0x7FFFFFF;
        output[j+3]  = ((encode[2] >> 17) | (encode[3] << 15)) & 0x7FFFFFF;
        output[j+4]  = ((encode[3] >> 12) | (encode[4] << 20)) & 0x7FFFFFF;
        output[j+5]  = ((encode[4] >> 7) | (encode[5] << 25)) & 0x7FFFFFF;
        output[j+6]  = (encode[5] >> 2) & 0x7FFFFFF;
        output[j+7]  = ((encode[5] >> 29) | (encode[6] << 3)) & 0x7FFFFFF;
        output[j+8]  = ((encode[6] >> 24) | (encode[7] << 8)) & 0x7FFFFFF;
        output[j+9]  = ((encode[7] >> 19) | (encode[8] << 13)) & 0x7FFFFFF;
        output[j+10] = ((encode[8] >> 14) | (encode[9] << 18)) & 0x7FFFFFF;
        output[j+11] = ((encode[9] >> 9) | (encode[10] << 23)) & 0x7FFFFFF;
        output[j+12] = (encode[10] >> 4) & 0x7FFFFFF;
        output[j+13] = ((encode[10] >> 31) | (encode[11] << 1)) & 0x7FFFFFF;
        output[j+14] = ((encode[11] >> 26) | (encode[12] << 6)) & 0x7FFFFFF;
        output[j+15] = ((encode[12] >> 21) | (encode[13] << 11)) & 0x7FFFFFF;
        output[j+16] = ((encode[13] >> 16) | (encode[14] << 16)) & 0x7FFFFFF;
        output[j+17] = ((encode[14] >> 11) | (encode[15] << 21)) & 0x7FFFFFF;
        output[j+18] = ((encode[15] >> 6) | (encode[16] << 26)) & 0x7FFFFFF;
        output[j+19] = (encode[16] >> 1) & 0x7FFFFFF;
        output[j+20] = ((encode[16] >> 28) | (encode[17] << 4)) & 0x7FFFFFF;
        output[j+21] = ((encode[17] >> 23) | (encode[18] << 9)) & 0x7FFFFFF;
        output[j+22] = ((encode[18] >> 18) | (encode[19] << 14)) & 0x7FFFFFF;
        output[j+23] = ((encode[19] >> 13) | (encode[20] << 19)) & 0x7FFFFFF;
        output[j+24] = ((encode[20] >> 8) | (encode[21] << 24)) & 0x7FFFFFF;
        output[j+25] = (encode[21] >> 3) & 0x7FFFFFF;
        output[j+26] = ((encode[21] >> 30) | (encode[22] << 2)) & 0x7FFFFFF;
        output[j+27] = ((encode[22] >> 25) | (encode[23] << 7)) & 0x7FFFFFF;
        output[j+28] = ((encode[23] >> 20) | (encode[24] << 12)) & 0x7FFFFFF;
        output[j+29] = ((encode[24] >> 15) | (encode[25] << 17)) & 0x7FFFFFF;
        output[j+30] = ((encode[25] >> 10) | (encode[26] << 22)) & 0x7FFFFFF;
        output[j+31] = (encode[26] >> 5) & 0x7FFFFFF;

        encode += 27;
        j      += 32;
    }
}

static void
p4d_unpack_28( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0xFFFFFFF;
        output[j+1]  = ((encode[0] >> 28) | (encode[1] << 4)) & 0xFFFFFFF;
        output[j+2]  = ((encode[1] >> 24) | (encode[2] << 8)) & 0xFFFFFFF;
        output[j+3]  = ((encode[2] >> 20) | (encode[3] << 12)) & 0xFFFFFFF;
        output[j+4]  = ((encode[3] >> 16) | (encode[4] << 16)) & 0xFFFFFFF;
        output[j+5]  = ((encode[4] >> 12) | (encode[5] << 20)) & 0xFFFFFFF;
        output[j+6]  = ((encode[5] >> 8) | (encode[6] << 24)) & 0xFFFFFFF;
        output[j+7]  = (encode[6] >> 4) & 0xFFFFFFF;
        output[j+8]  = encode[7] & 0xFFFFFFF;
        output[j+9]  = ((encode[7] >> 28) | (encode[8] << 4)) & 0xFFFFFFF;
        output[j+10] = ((encode[8] >> 24) | (encode[9] << 8)) & 0xFFFFFFF;
        output[j+11] = ((encode[9] >> 20) | (encode[10] << 12)) & 0xFFFFFFF;
        output[j+12] = ((encode[10] >> 16) | (encode[11] << 16)) & 0xFFFFFFF;
        output[j+13] = ((encode[11] >> 12) | (encode[12] << 20)) & 0xFFFFFFF;
        output[j+14] = ((encode[12] >> 8) | (encode[13] << 24)) & 0xFFFFFFF;
        output[j+15] = (encode[13] >> 4) & 0xFFFFFFF;
        output[j+16] = encode[14] & 0xFFFFFFF;
        output[j+17] = ((encode[14] >> 28) | (encode[15] << 4)) & 0xFFFFFFF;
        output[j+18] = ((encode[15] >> 24) | (encode[16] << 8)) & 0xFFFFFFF;
        output[j+19] = ((encode[16] >> 20) | (encode[17] << 12)) & 0xFFFFFFF;
        output[j+20] = ((encode[17] >> 16) | (encode[18] << 16)) & 0xFFFFFFF;
        output[j+21] = ((encode[18] >> 12) | (encode[19] << 20)) & 0xFFFFFFF;
        output[j+22] = ((encode[19] >> 8) | (encode[20] << 24)) & 0xFFFFFFF;
        output[j+23] = (encode[20] >> 4) & 0xFFFFFFF;
        output[j+24] = encode[21] & 0xFFFFFFF;
        output[j+25] = ((encode[21] >> 28) | (encode[22] << 4)) & 0xFFFFFFF;
        output[j+26] = ((encode[22] >> 24) | (encode[23] << 8)) & 0xFFFFFFF;
        output[j+27] = ((encode[23] >> 20) | (encode[24] << 12)) & 0xFFFFFFF;
        output[j+28] = ((encode[24] >> 16) | (encode[25] << 16)) & 0xFFFFFFF;
        output[j+29] = ((encode[25] >> 12) | (encode[26] << 20)) & 0xFFFFFFF;
        output[j+30] = ((encode[26] >> 8) | (encode[27] << 24)) & 0xFFFFFFF;
        output[j+31] = (encode[27] >> 4) & 0xFFFFFFF;

        encode += 28;
        j      += 32;
    }
}

static void
p4d_unpack_29( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x1FFFFFFF;
        output[j+1]  = ((encode[0] >> 29) | (encode[1] << 3)) & 0x1FFFFFFF;
        output[j+2]  = ((encode[1] >> 26) | (encode[2] << 6)) & 0x1FFFFFFF;
        output[j+3]  = ((encode[2] >> 23) | (encode[3] << 9)) & 0x1FFFFFFF;
        output[j+4]  = ((encode[3] >> 20) | (encode[4] << 12)) & 0x1FFFFFFF;
        output[j+5]  = ((encode[4] >> 17) | (encode[5] << 15)) & 0x1FFFFFFF;
        output[j+6]  = ((encode[5] >> 14) | (encode[6] << 18)) & 0x1FFFFFFF;
        output[j+7]  = ((encode[6] >> 11) | (encode[7] << 21)) & 0x1FFFFFFF;
        output[j+8]  = ((encode[7] >> 8) | (encode[8] << 24)) & 0x1FFFFFFF;
        output[j+9]  = ((encode[8] >> 5) | (encode[9] << 27)) & 0x1FFFFFFF;
        output[j+10] = (encode[9] >> 2) & 0x1FFFFFFF;
        output[j+11] = ((encode[9] >> 31) | (encode[10] << 1)) & 0x1FFFFFFF;
        output[j+12] = ((encode[10] >> 28) | (encode[11] << 4)) & 0x1FFFFFFF;
        output[j+13] = ((encode[11] >> 25) | (encode[12] << 7)) & 0x1FFFFFFF;
        output[j+14] = ((encode[12] >> 22) | (encode[13] << 10)) & 0x1FFFFFFF;
        output[j+15] = ((encode[13] >> 19) | (encode[14] << 13)) & 0x1FFFFFFF;
        output[j+16] = ((encode[14] >> 16) | (encode[15] << 16)) & 0x1FFFFFFF;
        output[j+17] = ((encode[15] >> 13) | (encode[16] << 19)) & 0x1FFFFFFF;
        output[j+18] = ((encode[16] >> 10) | (encode[17] << 22)) & 0x1FFFFFFF;
        output[j+19] = ((encode[17] >> 7) | (encode[18] << 25)) & 0x1FFFFFFF;
        output[j+20] = ((encode[18] >> 4) | (encode[19] << 28)) & 0x1FFFFFFF;
        output[j+21] = (encode[19] >> 1) & 0x1FFFFFFF;
        output[j+22] = ((encode[19] >> 30) | (encode[20] << 2)) & 0x1FFFFFFF;
        output[j+23] = ((encode[20] >> 27) | (encode[21] << 5)) & 0x1FFFFFFF;
        output[j+24] = ((encode[21] >> 24) | (encode[22] << 8)) & 0x1FFFFFFF;
        output[j+25] = ((encode[22] >> 21) | (encode[23] << 11)) & 0x1FFFFFFF;
        output[j+26] = ((encode[23] >> 18) | (encode[24] << 14)) & 0x1FFFFFFF;
        output[j+27] = ((encode[24] >> 15) | (encode[25] << 17)) & 0x1FFFFFFF;
        output[j+28] = ((encode[25] >> 12) | (encode[26] << 20)) & 0x1FFFFFFF;
        output[j+29] = ((encode[26] >> 9) | (encode[27] << 23)) & 0x1FFFFFFF;
        output[j+30] = ((encode[27] >> 6) | (encode[28] << 26)) & 0x1FFFFFFF;
        output[j+31] = (encode[28] >> 3) & 0x1FFFFFFF;

        encode += 29;
        j      += 32;
    }
}

static void
p4d_unpack_30( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x3FFFFFFF;
        output[j+1]  = ((encode[0] >> 30) | (encode[1] << 2)) & 0x3FFFFFFF;
        output[j+2]  = ((encode[1] >> 28) | (encode[2] << 4)) & 0x3FFFFFFF;
        output[j+3]  = ((encode[2] >> 26) | (encode[3] << 6)) & 0x3FFFFFFF;
        output[j+4]  = ((encode[3] >> 24) | (encode[4] << 8)) & 0x3FFFFFFF;
        output[j+5]  = ((encode[4] >> 22) | (encode[5] << 10)) & 0x3FFFFFFF;
        output[j+6]  = ((encode[5] >> 20) | (encode[6] << 12)) & 0x3FFFFFFF;
        output[j+7]  = ((encode[6] >> 18) | (encode[7] << 14)) & 0x3FFFFFFF;
        output[j+8]  = ((encode[7] >> 16) | (encode[8] << 16)) & 0x3FFFFFFF;
        output[j+9]  = ((encode[8] >> 14) | (encode[9] << 18)) & 0x3FFFFFFF;
        output[j+10] = ((encode[9] >> 12) | (encode[10] << 20)) & 0x3FFFFFFF;
        output[j+11] = ((encode[10] >> 10) | (encode[11] << 22)) & 0x3FFFFFFF;
        output[j+12] = ((encode[11] >> 8) | (encode[12] << 24)) & 0x3FFFFFFF;
        output[j+13] = ((encode[12] >> 6) | (encode[13] << 26)) & 0x3FFFFFFF;
        output[j+14] = ((encode[13] >> 4) | (encode[14] << 28)) & 0x3FFFFFFF;
        output[j+15] =  (encode[14] >> 2) & 0x3FFFFFFF;
        output[j+16] =   encode[15] & 0x3FFFFFFF;
        output[j+17] = ((encode[15] >> 30) | (encode[16] << 2)) & 0x3FFFFFFF;
        output[j+18] = ((encode[16] >> 28) | (encode[17] << 4)) & 0x3FFFFFFF;
        output[j+19] = ((encode[17] >> 26) | (encode[18] << 6)) & 0x3FFFFFFF;
        output[j+20] = ((encode[18] >> 24) | (encode[19] << 8)) & 0x3FFFFFFF;
        output[j+21] = ((encode[19] >> 22) | (encode[20] << 10)) & 0x3FFFFFFF;
        output[j+22] = ((encode[20] >> 20) | (encode[21] << 12)) & 0x3FFFFFFF;
        output[j+23] = ((encode[21] >> 18) | (encode[22] << 14)) & 0x3FFFFFFF;
        output[j+24] = ((encode[22] >> 16) | (encode[23] << 16)) & 0x3FFFFFFF;
        output[j+25] = ((encode[23] >> 14) | (encode[24] << 18)) & 0x3FFFFFFF;
        output[j+26] = ((encode[24] >> 12) | (encode[25] << 20)) & 0x3FFFFFFF;
        output[j+27] = ((encode[25] >> 10) | (encode[26] << 22)) & 0x3FFFFFFF;
        output[j+28] = ((encode[26] >> 8) | (encode[27] << 24)) & 0x3FFFFFFF;
        output[j+29] = ((encode[27] >> 6) | (encode[28] << 26)) & 0x3FFFFFFF;
        output[j+30] = ((encode[28] >> 4) | (encode[29] << 28)) & 0x3FFFFFFF;
        output[j+31] =  (encode[29] >> 2) & 0x3FFFFFFF;

        encode += 30;
        j      += 32;
    }
}

static void
p4d_unpack_31( uint32_t * output,  const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0] & 0x7FFFFFFF;
        output[j+1]  = ((encode[0] >> 31) | (encode[1] << 1)) & 0x7FFFFFFF;
        output[j+2]  = ((encode[1] >> 30) | (encode[2] << 2)) & 0x7FFFFFFF;
        output[j+3]  = ((encode[2] >> 29) | (encode[3] << 3)) & 0x7FFFFFFF;
        output[j+4]  = ((encode[3] >> 28) | (encode[4] << 4)) & 0x7FFFFFFF;
        output[j+5]  = ((encode[4] >> 27) | (encode[5] << 5)) & 0x7FFFFFFF;
        output[j+6]  = ((encode[5] >> 26) | (encode[6] << 6)) & 0x7FFFFFFF;
        output[j+7]  = ((encode[6] >> 25) | (encode[7] << 7)) & 0x7FFFFFFF;
        output[j+8]  = ((encode[7] >> 24) | (encode[8] << 8)) & 0x7FFFFFFF;
        output[j+9]  = ((encode[8] >> 23) | (encode[9] << 9)) & 0x7FFFFFFF;
        output[j+10] = ((encode[9] >> 22) | (encode[10] << 10)) & 0x7FFFFFFF;
        output[j+11] = ((encode[10] >> 21) | (encode[11] << 11)) & 0x7FFFFFFF;
        output[j+12] = ((encode[11] >> 20) | (encode[12] << 12)) & 0x7FFFFFFF;
        output[j+13] = ((encode[12] >> 19) | (encode[13] << 13)) & 0x7FFFFFFF;
        output[j+14] = ((encode[13] >> 18) | (encode[14] << 14)) & 0x7FFFFFFF;
        output[j+15] = ((encode[14] >> 17) | (encode[15] << 15)) & 0x7FFFFFFF;
        output[j+16] = ((encode[15] >> 16) | (encode[16] << 16)) & 0x7FFFFFFF;
        output[j+17] = ((encode[16] >> 15) | (encode[17] << 17)) & 0x7FFFFFFF;
        output[j+18] = ((encode[17] >> 14) | (encode[18] << 18)) & 0x7FFFFFFF;
        output[j+19] = ((encode[18] >> 13) | (encode[19] << 19)) & 0x7FFFFFFF;
        output[j+20] = ((encode[19] >> 12) | (encode[20] << 20)) & 0x7FFFFFFF;
        output[j+21] = ((encode[20] >> 11) | (encode[21] << 21)) & 0x7FFFFFFF;
        output[j+22] = ((encode[21] >> 10) | (encode[22] << 22)) & 0x7FFFFFFF;
        output[j+23] = ((encode[22] >> 9) | (encode[23] << 23)) & 0x7FFFFFFF;
        output[j+24] = ((encode[23] >> 8) | (encode[24] << 24)) & 0x7FFFFFFF;
        output[j+25] = ((encode[24] >> 7) | (encode[25] << 25)) & 0x7FFFFFFF;
        output[j+26] = ((encode[25] >> 6) | (encode[26] << 26)) & 0x7FFFFFFF;
        output[j+27] = ((encode[26] >> 5) | (encode[27] << 27)) & 0x7FFFFFFF;
        output[j+28] = ((encode[27] >> 4) | (encode[28] << 28)) & 0x7FFFFFFF;
        output[j+29] = ((encode[28] >> 3) | (encode[29] << 29)) & 0x7FFFFFFF;
        output[j+30] = ((encode[29] >> 2) | (encode[30] << 30)) & 0x7FFFFFFF;
        output[j+31] = (encode[30] >> 1) & 0x7FFFFFFF;

        encode += 31;
        j      += 32;
    }
}


static void
p4d_unpack_32( uint32_t * output, const uint32_t * encode, int num )
{
    register int j = 0;

    for ( ; num >= 32; num -= 32 )
    {
        output[j+0]  = encode[0]  & 0xFFFFFFFF;
        output[j+1]  = encode[1]  & 0xFFFFFFFF;
        output[j+2]  = encode[2]  & 0xFFFFFFFF;
        output[j+3]  = encode[3]  & 0xFFFFFFFF;
        output[j+4]  = encode[4]  & 0xFFFFFFFF;
        output[j+5]  = encode[5]  & 0xFFFFFFFF;
        output[j+6]  = encode[6]  & 0xFFFFFFFF;
        output[j+7]  = encode[7]  & 0xFFFFFFFF;
        output[j+8]  = encode[8]  & 0xFFFFFFFF;
        output[j+9]  = encode[9]  & 0xFFFFFFFF;
        output[j+10] = encode[10] & 0xFFFFFFFF;
        output[j+11] = encode[11] & 0xFFFFFFFF;
        output[j+12] = encode[12] & 0xFFFFFFFF;
        output[j+13] = encode[13] & 0xFFFFFFFF;
        output[j+14] = encode[14] & 0xFFFFFFFF;
        output[j+15] = encode[15] & 0xFFFFFFFF;
        output[j+16] = encode[16] & 0xFFFFFFFF;
        output[j+17] = encode[17] & 0xFFFFFFFF;
        output[j+18] = encode[18] & 0xFFFFFFFF;
        output[j+19] = encode[19] & 0xFFFFFFFF;
        output[j+20] = encode[20] & 0xFFFFFFFF;
        output[j+21] = encode[21] & 0xFFFFFFFF;
        output[j+22] = encode[22] & 0xFFFFFFFF;
        output[j+23] = encode[23] & 0xFFFFFFFF;
        output[j+24] = encode[24] & 0xFFFFFFFF;
        output[j+25] = encode[25] & 0xFFFFFFFF;
        output[j+26] = encode[26] & 0xFFFFFFFF;
        output[j+27] = encode[27] & 0xFFFFFFFF;
        output[j+28] = encode[28] & 0xFFFFFFFF;
        output[j+29] = encode[29] & 0xFFFFFFFF;
        output[j+30] = encode[30] & 0xFFFFFFFF;
        output[j+31] = encode[31] & 0xFFFFFFFF;

        encode += 32;
        j      += 32;
    }
}




/** 进行压缩 */
int  p4d_compress( uint32_t * in_arr, int num, uint8_t * buf )
{
    p4d_delta( in_arr, num );                                        // 转换成差值形式

    int        bit_size = p4d_getBitSize( in_arr, num );             // 计算压缩位数
    uint32_t   exc_min  = (1u << bit_size) - 1;                      // 超过这个数，当做异常值
    int        exc_oft  = align_int( bit_size * (num + 1), 8 ) >> 3; // 异常值的写入位置
    uint8_t  * entry    = buf + 1;                                   // 正常值的起始写入位置
    uint8_t  * excepts  = entry + exc_oft;                           // 异常值的起始写入位置
    uint32_t * encode   = (uint32_t *)( entry );                     // 正常值的起始写入位置
    int        ec_num   = align_int( exc_oft, 4 ) >> 2;              // 正常值的个数
    int        last_idx = -1;                                        // 上一个异常值写入的位置
    int        idx      = 0;                                         // 当前异常值可写入位置

    /** 记录每个正常值单元的bit数   */
    buf[ 0 ] = (uint8_t)bit_size;

    for ( int i = 0; i < ec_num; ++i )                    // 先把正常值的位置都设置成0
    {
        encode[ i ] = 0;
    }

    for ( int j = 0; j < num; ++j )
    {
        if ( in_arr[ j ] > exc_min || (uint32_t)(idx - last_idx) >= exc_min )
        {                                                  // 处理异常值
            if ( -1 == last_idx )
            {
                *encode = encode[ 0 ] | (idx + 1);
            }
            else
            {
                p4d_writeto( idx - last_idx, encode, bit_size, last_idx );
            }

            last_idx = idx + 1;

            // 记录异常值
            excepts = varint_encode_uint32( in_arr[ j ], excepts );
            //*(uint32_t *)excepts  = in_arr[ j ];
            //             excepts += sizeof( uint32_t );
        }
        else                                               // 处理正常值
        {
            p4d_writeto( in_arr[ j ], encode, bit_size, idx + 1 );
        }

        idx++;
    }

    if ( last_idx != -1 )
    {
        p4d_writeto( exc_min, encode, bit_size, last_idx );
    }
    else
    {
        *entry |= (uint8_t)exc_min;
    }

    p4d_deDelta( in_arr, num );                            // 将差值形式还原

    return excepts - buf;
}





/** 解压 完整 的   P4D_BLOCK_SIZE 个 数字 */
const uint8_t *
p4d_uncompress( const uint8_t * cmp_data, int num, uint32_t * out_arr )
{
    int              bit_size = cmp_data[ 0 ];
    int              exc_oft  = align_int( bit_size * (num + 1), 8 ) >> 3;
    const uint8_t  * excepts  = cmp_data + 1 + exc_oft;              // 异常值数组起始位置
    const uint32_t * codes    = (uint32_t *)(cmp_data + 1);          // 正常值数组起始位置

    /**************** 解压 ******************/
    switch ( bit_size )
    {
        case 4:  p4d_unpack_4 ( out_arr, codes, align_int( num, 32 ) ); break;
        case 5:  p4d_unpack_5 ( out_arr, codes, align_int( num, 32 ) ); break;
        case 6:  p4d_unpack_6 ( out_arr, codes, align_int( num, 32 ) ); break;
        case 7:  p4d_unpack_7 ( out_arr, codes, align_int( num, 32 ) ); break;
        case 8:  p4d_unpack_8 ( out_arr, codes, align_int( num, 32 ) ); break;
        case 9:  p4d_unpack_9 ( out_arr, codes, align_int( num, 32 ) ); break;
        case 10: p4d_unpack_10( out_arr, codes, align_int( num, 32 ) ); break;
        case 11: p4d_unpack_11( out_arr, codes, align_int( num, 32 ) ); break;
        case 12: p4d_unpack_12( out_arr, codes, align_int( num, 32 ) ); break;
        case 13: p4d_unpack_13( out_arr, codes, align_int( num, 32 ) ); break;
        case 14: p4d_unpack_14( out_arr, codes, align_int( num, 32 ) ); break;
        case 15: p4d_unpack_15( out_arr, codes, align_int( num, 32 ) ); break;
        case 16: p4d_unpack_16( out_arr, codes, align_int( num, 32 ) ); break;
        case 17: p4d_unpack_17( out_arr, codes, align_int( num, 32 ) ); break;
        case 18: p4d_unpack_18( out_arr, codes, align_int( num, 32 ) ); break;
        case 19: p4d_unpack_19( out_arr, codes, align_int( num, 32 ) ); break;
        case 1:  p4d_unpack_1 ( out_arr, codes, align_int( num, 32 ) ); break;
        case 2:  p4d_unpack_2 ( out_arr, codes, align_int( num, 32 ) ); break;
        case 3:  p4d_unpack_3 ( out_arr, codes, align_int( num, 32 ) ); break;
        case 20: p4d_unpack_20( out_arr, codes, align_int( num, 32 ) ); break;
        case 21: p4d_unpack_21( out_arr, codes, align_int( num, 32 ) ); break;
        case 22: p4d_unpack_22( out_arr, codes, align_int( num, 32 ) ); break;
        case 23: p4d_unpack_23( out_arr, codes, align_int( num, 32 ) ); break;
        case 24: p4d_unpack_24( out_arr, codes, align_int( num, 32 ) ); break;
        case 25: p4d_unpack_25( out_arr, codes, align_int( num, 32 ) ); break;
        case 26: p4d_unpack_26( out_arr, codes, align_int( num, 32 ) ); break;
        case 27: p4d_unpack_27( out_arr, codes, align_int( num, 32 ) ); break;
        case 28: p4d_unpack_28( out_arr, codes, align_int( num, 32 ) ); break;
        case 29: p4d_unpack_29( out_arr, codes, align_int( num, 32 ) ); break;
        case 30: p4d_unpack_30( out_arr, codes, align_int( num, 32 ) ); break;
        case 31: p4d_unpack_31( out_arr, codes, align_int( num, 32 ) ); break;
        default: p4d_unpack_32( out_arr, codes, align_int( num, 32 ) );
    }

    /**************** 处理异常值  **************/
    uint32_t  exc_next = out_arr[ 0 ];                     // 和下一个异常的距离
              exc_oft  = exc_next;                         // 异常值的索引位置

    while ( exc_oft <= num )
    {
        exc_next = out_arr[ exc_oft ] + 1;

        excepts  = varint_decode_uint32( excepts, out_arr[ exc_oft ] );

        // out_arr[ exc_oft ]  = *(uint32_t *) excepts;
        //            excepts += sizeof(uint32_t);

        exc_oft += exc_next;
    }

    /******** 将差值还原， 数组的第一个值 丢弃, 只有127个值   ********/
    uint32_t  total     = 0;
    int       align_num = num & 0xFFFFFFFC;

    for ( int i = 0; i < align_num; i +=4 )
    {
        uint32_t  arr0  = out_arr[ i + 1 ];
        uint32_t  arr1  = out_arr[ i + 2 ];
        uint32_t  arr2  = out_arr[ i + 3 ];
        uint32_t  arr3  = out_arr[ i + 4];
        uint32_t  half1 = arr0 + arr1;
        uint32_t  half2 = arr2 + arr3;

        out_arr[ i ]     = total + arr0;
        out_arr[ i + 1 ] = total + half1;
        out_arr[ i + 2 ] = total + half1 + arr2;
        out_arr[ i + 3 ] = total + half1 + half2;
        total            += half1 + half2;
    }

    for ( int i = align_num; i < num; ++i )
    {
        uint32_t  arr0 = out_arr[ i + 1 ];

        out_arr[ i ]  = total + arr0;
        total        += arr0;
    }

    return excepts;
}



}
