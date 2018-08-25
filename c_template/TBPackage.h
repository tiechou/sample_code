/**
 * 打包和解包工具：把多个name/value对合成一个包 或 把包解析成name/value对，包可以用于网络传输
 * @author Xu Lubing
 * @date Mar. 10, 2009
 */
#ifndef _PACKAGER_H_
#define _PACKAGER_H_

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>

namespace TB_Ad_Common
{

// 各种错误码
enum EErrCode
{
	EC_NO_ERROR         = 0,
	EC_DUPLICATE_NAME   = 1, // 重复命名
	EC_NAME_NIL         = 2, // NAME指向的地址为NULL
	EC_VAL_NIL          = 3, // VALUE指向的地址为NULL
	EC_INVALID_VAL_SIZE = 4, // value字节数非法
	EC_PACK_NIL         = 5, // 需要解包的package没有内容
	EC_INVALID_PACK_SIZE= 6, // 需要解包的字节数非法
	EC_INVALID_PACKAGE  = 7, // 非法的package
	EC_UNKNOWN_PACK_VER = 8, // 未知的package版本
	EC_NAME_BUFFER_NIL  = 9, // 存放name的缓冲区为NULL
	EC_VAL_BUFFER_NIL   = 10,// 存放value的缓冲区为NULL
	EC_VAL_SIZE_NIL     = 11,// 存放value长度的地址为NULL
	EC_NAME_NOT_FOUND   = 12,// 没有指定的name对应的value
	EC_MEM_ALLOC_FAILED = 13,// 申请内存失败
	EC_NO_MORE_NV_PAIR  = 14,// 没有name/value了
	EC_PACK_NOT_MATCHED = 15,// 调用了不配套的package函数
	
	TOAL_EC
};

struct SNV;                         // name/value对，内部使用
typedef std::vector<SNV>  TNVPairs; // name/value对集合

// 不区分大小写比较类，定义TNameIndexMap使用
struct SStrCaseCmp
{
	bool operator()(const char * const &s1, const char * const &s2) const
	{
		return (strcasecmp(s1, s2) < 0);
	}
	
	bool operator()(const std::string &s1, const std::string &s2) const
	{
		return (strcasecmp(s1.c_str(), s2.c_str()) < 0);
	}
};
typedef std::map<const char*, size_t, SStrCaseCmp> TNameIndexMap; // name => 下标, name不区分大小写
typedef std::set<std::string, SStrCaseCmp>         TNameSet;

/**
 * 打包工具：把多个name/value对合成一个包，包可以用于网络传输
 */
class CPackager
{
public:
	/**
	 * 带存放打包结果的构造函数
	 * @param res  用于存放打包结果
	 * [注]在使用后续的addNVPair时可以释放所引用的name和value空间
	 *     调用者自己不要修改res值
	 *     通过无参数的package()获得打包结果
	 */ 
	CPackager(std::string &res);
	
	/** 析构函数 */
	virtual ~CPackager();
	
	/**
 	 * 添加一个name/value对
	 * @param name    name名称
	 * @param valLen  val的字节数(val类型为void*时需要提供)
	 * @param val     value值，如果value是一个复杂的结构体，建议先做序列化
	 * @param valType val的数据类型(缺省为0，表示未知，其它值根据需要自行指明)
	 * @return  0:成功；<0:各种异常
	 * [注]1. 如果用无参的CPackager，在没有调用package前，不要释放name和val指向的数据
	 *     2. 如果使用void*传入val，valSize参数出现在val前
 	 */
	int addNVPair(const std::string &name, const std::string &val, char valType=0);
	int addNVPair(const std::string &name, const char *val, char valType=0);
	int addNVPair(const std::string &name, size_t valSize, const void *val, char valType=0);
	int addNVPair(const char *name, const std::string &val, char valType=0);
	int addNVPair(const char *name, const char *val, char valType=0);
	int addNVPair(const char *name, size_t valSize, const void *val, char valType=0);
	
	/**
	 * 把所有的name/val对打包，和参数的构造函数对应
	 * @return 0:成功; <0: 各种异常
 	 */
	int package();
	
	/**
	 * 清除打包状态。后续支持新的addNVPair
 	 */
	void clear();

protected:
	std::string       &m_res;     // 保存带参数构造器的结果
	
	std::vector<SNV>   m_nvs;     // 保存所有通过addNVPair加入的name/value对
	TNameSet           m_nameSet; // 当m_res!=NULL时使用，避免name重复
	
private:
	int addNVPair(const char *name, size_t nameSize, const void *val, size_t valSize, char valType);
};

struct SNVPos; // 用于定位name/value的结构

/**
 * 对通过网络传输获得的用CPackager打包的内容进行解析
 */
class CUnpackager
{
public:
	/**
	 * 空构造函数，需要解包的话，可以调用unpackage()
	 */
	CUnpackager();
	/**
	 * 构造函数，对指定的包进行解析
	 * @param pack      需要解些的包
	 * @param packSize  pack的字节数(pack类型为void*需要提供)
	 */
	CUnpackager(const std::string &pack);
	CUnpackager(const void *pack, size_t packSize);

	/** 析构函数 */
	virtual ~CUnpackager();
	
	/**
	 * 解析一个数据包
	 * @param pack      需要解些的包
	 * @param packSize  pack的字节数(pack类型为void*需要提供)
	 * @return 0:成功; <0:各种异常
	 */
	int unpackage(const std::string &pack);
	int unpackage(const void *pack, size_t packSize);
	
	/**
	 * 获取指定name的值
	 * @param name     指定的name
	 * @param val      保存name对应的值
	 * @param valSize  保存val中的有效字节数(当val的类型为void**时需要提供)
	 * @param valType  val的类型(可以抛弃，类型根据需要自行定义，参见CPackager::addNVPair)
	 * @return 0:成功; <0:各种异常
	 */
	int getValue(const std::string &name, std::string &val, char *valType=NULL);
	int getValue(const char *name, std::string &val, char *valType=NULL);
	int getValue(const std::string &name, void **val, size_t *valSize, char *valType=NULL);
	int getValue(const char *name, void **val, size_t *valSize, char *valType=NULL);

	/**
	 * 获取下一个name/value对
	 * @param name     保存名称
	 * @param val      保存value值
	 * @param valSize  保存val中的有效字节数(当val的类型为void**时需要提供)
	 * @param valType  val的类型(可以抛弃，类型根据需要自行定义，参见CPackager::addNVPair)
	 * @return 0:成功; <0:异常或者没有name/value对了
 	 */	
	int getNextNVPair(std::string &name, std::string &val, char *valType=NULL);
	int getNextNVPair(char **name, void **val, size_t *valSize, char *valType=NULL);

protected:
	bool           m_valid;       // 包是否合法?
	const void    *m_pack;        // 保存需要解析的包
	size_t         m_packSize;    // pack的字节数
	const char    *m_packBody;    // pack中包体的起始地址
	
	size_t         m_nvCount;     // name/value个数
	SNVPos        *m_nvPos;       // 存放name/value在m_packBody中的位置信息
	size_t         m_nextPairIdx; // 记录调用getNextNVPair时使用的m_pack的下标
	
	TNameIndexMap  m_nameIdx;     // name => index in m_nvPos，加速查找

private:
	void getValueByIdx(size_t i, void **val, size_t *valSize, char *valType); // 根据下标取value
};

}

#endif

