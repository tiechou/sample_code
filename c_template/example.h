/** \file
 ********************************************************************
 * $Author: luoli.hn $
 *
 * $LastChangedBy: luoli.hn $
 *
 * $Revision: 476 $
 *
 * $LastChangedDate: 2011-04-08 15:15:34 +0800 (Fri, 08 Apr 2011) $
 *
 * $Id: FloatAttrFilter.h 476 2011-04-08 07:15:34Z luoli.hn $
 *
 * $Brief: 
 *******************************************************************
 */
#ifndef _UPS_CLIENT_API_H
#define _UPS_CLIENT_API_H


namespace anet {
class DefaultPacket;
class Transport;
}
namespace client_api {

const int32_t k_task_queue = 222; //connection_pool队列长度

enum node_type {
	timeout;
	unavalible;
}
struct ParseConfig {
    string s_table_name; //表名
	int32_t n_time_out; //超时时间
	int32_t n_task_queue; //任务队列的长度
	char* p_log_file; //log 输出文件
	node_type enum_node_type;//结点信息
}


	/**
	 *封装UPS的查询接口
	 *
	 **/
class ClientApi	{
  prublic:
	ClientApi();
	~ClientApi();
	//初始化 ClientApi
	bool initClientApi(const int32_t connect_time, 
	const string table_name, 
	const string primary_key, 
    anet::Transport* transport, 
	const vector<pthread_t> &vec_thread_ids);
	//和UPS的某一台机器建立连接
	bool makeConnectionToUps(void);
	void setConnectTime(int32_t connect_time);
	int32_t getConnectTime(void);

  protect:
  private:
    int32_t _n_connect_time;
	string  _s_table_name; // 表名  
	string  _s_primary_key;
	char*   _p_result; //UPS返回的结果
	anet::Transport*  _transport;
	vector<pthread_t> _vec_thread_ids;//线程ID的vectort

	}
}
