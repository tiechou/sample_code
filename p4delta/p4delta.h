/******************************************************************
 *  Created on: 2013-4-16
 *      Author: yewang@taobao.com clm971910@gmail.com
 *
 *      Desc  : 提供 p4delta的  压缩和解压 功能
 *
 ******************************************************************/


#ifndef P4DELTA_H
#define P4DELTA_H


#define P4D_BLOCK_SIZE     127
#define BOUND_RATIO        0.9
#define EXCEPTION_BYTE     4


namespace ups_util
{


/**
 *  压缩入口, 将传入的整数数组 压缩成一个字节流  , 线程安全
 *
 * @param  in_arr   传入的整数数组,
 *                  数组会被修改后还原, 这个数组如果被别的线程读 可能出错
 *
 * @param  num      整数的个数, 必须 <= P4D_BLOCK_SIZE
 *
 * @param  buf      用于存储压缩结果, 长度必须 >=((num + 1) * 8)
 *                  内部不会检查边界，请使用方注意
 *
 * @return  返回的压缩字节数
 */
int p4d_compress( uint32_t * in_arr, int num, uint8_t * buf );



/**
 * 解压缩入口, 只解压   block_size 个整数, 线程安全
 *
 * @param   cmp_data     输入的压缩字节流
 * @param   num          要解压的整数个数   <= P4D_BLOCK_SIZE
 * @param   out_arr      解压的整数数组
 *                       数组必须  block_size + 1 按32向上取整 的 个容量
 *
 * @return  返回下一个可解压位置
 */
const uint8_t *
p4d_uncompress( const uint8_t * cmp_data, int num, uint32_t * out_arr );


}

#endif
