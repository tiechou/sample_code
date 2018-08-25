/*
 * =====================================================================================
 * 
 *       Filename:  example.cpp
 * 
 *    Description:  This is a copy from tiechou; anet server application just to show how
 *                  simple it can be to build a server application using anet libarary. It
 *                  listens to 0.0.0.0:5555 to accept incoming connections from any number
 *                  of client. Once a connection between client and helloworld_s establised,
 *                  the client can send request to helloworld_s through anet libarary, and
 *                  the helloworld_s will get that request and reverse the request and send
 *                  it back to the client.
 * 
 *        Version:  1.0
 *        Created:  2011-08-04 15:11:09
 * LastChangeDate:  2011-09-04 12:34:34
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  luoli.hn(search engine group), luoli.hn@taobao.com
 *        Company:  www.taobao.com
 * 
 * =====================================================================================
 */

#include <stdint.h>
#include <iostream>
#include <ctime>
#include "ups/client/api/example.h"

namespace client_api {
/**
 *ClientApi的构造函数
 *初始化tranport以及result
 */
ClientApi() {
    _p_result = NULL;
	_transport = NULL;
}

/**
 *@param n_time:当前的时间戳; p_log_file:log的输出文件
 *如果线程vector的大小小于10,返回false
 *如果当前时间大于连接时间(connet_time),返回false
 *@return true or false
 */
bool makeConnectionToUps(const int32_t n_time, const char* p_log_file) {
	
	if( _transport != NULL 
		&& _vec_thread_ids.size() > 0
		&& _n_connect_time >= n_time) {
		//TODO:如果_transport为NULL,就意味着无法调用_transport->start()
		return false;
	}
	else {
		uint32_t n_task_queue = _n_connect_time + 10;
		uint32_t n_len = sizeof(n_task_queue); //sizeof 这里得用变量名称哦
		if( n_task_queue < 20 && vec_thread_ids.size > 10) {
			//监听socket
			_transport->start();
		}
		return true;
	}
}
}
