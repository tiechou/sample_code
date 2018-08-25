/**
 * 打包和解包工具：把多个name/value对合成一个包 或 把包解析成name/value对，包可以用于网络传输
 * 包的格式：
 *   <包体><meta信息>
 * 其中<meta信息>格式
 *   <n1长度><v1长度><n2长度><v2长度>...<n_n长度><v_n长度><版本><nv对数目>
 *   [以上数据全部为4字节整型，网络字节顺序]
 * <包体>格式
 *   <name1>\0<val1>\0<type1><name2>\0<val2>\0<type2>...<name_n>\0<value_n>\0<type_n>
 *
 * @author Xu Lubing
 * @date Mar. 10, 2009
 */
#include "TBPackage.h"
#include <arpa/inet.h>

namespace TB_Ad_Common
{

using namespace std;

// 常量定义
const int32_t PACK_VERSION  = 0x00010000; // 版本号, 4字节，高2字节为主版本号，低2字节为次版本号
const char    PACK_DELI     = '\0';       // 包内部使用的分隔符，主要是为了应用处理方便

// name/value对信息
struct SNV
{
	size_t      nameSize;
	size_t      valSize;

	SNV(size_t nameSize, size_t valSize) :
		nameSize(nameSize), valSize(valSize) {}
};

// 字符串辅助类，支持追加int 和 2进制数据
class CStrHelper
{
public:
	CStrHelper(string& s) : m_str(s) {}

	inline CStrHelper& operator+=(const string &s)  { m_str += s; return *this; }
	inline CStrHelper& operator+=(const char* s)    { m_str += s; return *this; }
	inline CStrHelper& operator+=(const char c)     { m_str += c; return *this; }
	inline CStrHelper& operator+=(const int32_t i)
	{
		// 转换成网络字节顺序，追加到string
		int32_t ni = htonl(i);
		m_str.append((const char*)&ni, sizeof(ni));
		return *this;
	}
	
	// 64位截断为32位
	inline CStrHelper& operator+=(const size_t i)   { return this->operator+=((const int32_t)i); } 
	
	inline CStrHelper& append(const char *v, size_t size) { m_str.append(v, size); return *this; }
	inline CStrHelper& append(const void *v, size_t size) { m_str.append((const char*)v, size); return *this; }

	inline string& operator*() { return m_str; }
	inline void clear() { m_str.clear(); }
	inline void reserve(size_t num) { m_str.reserve(num); }
private:
	string& m_str;
};

/*------------  打包工具：把多个name/value对合成一个包，包可以用于网络传输 -----------*/

/**
 * 带存放打包结果的构造函数
 * @param res  用于存放打包结果
 */
CPackager::CPackager(std::string &res) : m_res(res)
{
	m_res.clear();
}

/** 析构函数 */
CPackager::~CPackager()
{
}

void CPackager::clear()
{
	m_res.clear();
	m_nvs.clear();
	m_nameSet.clear();
}

/**
 * 添加一个name/value对
 * @param name    name名称
 * @param valLen  val的字节数(val类型为void*时需要提供)
 * @param val     value值，如果value是一个复杂的结构体，建议先做序列化
 * @param valType val的数据类型(缺省为0，表示未知，其它值根据需要自行指明)
 * @return  0:成功；<0:各种异常
 */
int CPackager::addNVPair(const std::string &name, const std::string &val, char valType)
{
	if (name.empty())
		return EC_NAME_NIL;
	
	return addNVPair(name, val.length(), val.c_str(), valType);
}

int CPackager::addNVPair(const std::string &name, const char *val, char valType)
{
	if (name.empty())
		return EC_NAME_NIL;
	size_t valSize = (val == NULL) ? 0 : strlen(val);
	
	return addNVPair(name, valSize, val, valType);
}

int CPackager::addNVPair(const std::string &name, size_t valSize, const void *val, char valType)
{
	if (name.empty())
		return EC_NAME_NIL;
	if (valSize < 0)
		return EC_INVALID_VAL_SIZE;
	/*
	if (val == NULL)
		return EC_VAL_NIL;
	*/
	
	// 检查是否重名
	TNameSet::iterator i = m_nameSet.find(name);
	if (i != m_nameSet.end())
		return EC_DUPLICATE_NAME;
	
	// 直接打包到结果中
	CStrHelper str(m_res);
	str += name;
	str += '\0';
	str.append((const char*)val, valSize);
	str += '\0';
	str += valType;
	
	// 保存name/value的地址信息和类型
	m_nameSet.insert(name);
	
	SNV nv(name.length(), valSize);
	m_nvs.push_back(nv);
	
	return 0;
}

int CPackager::addNVPair(const char *name, const std::string &val, char valType)
{
	if (name == NULL || name[0] == '\0')
		return EC_NAME_NIL;
	
	return addNVPair(name, strlen(name), val.c_str(), val.length(), valType);
}

int CPackager::addNVPair(const char *name, const char *val, char valType)
{
	if (name == NULL || name[0] == '\0')
		return EC_NAME_NIL;
	size_t valSize = (val == NULL) ? 0 : strlen(val);
	
	return addNVPair(name, strlen(name), val, valSize, valType);
}

int CPackager::addNVPair(const char *name, size_t valSize, const void *val, char valType)
{
	if (name == NULL || name[0] == '\0')
		return EC_NAME_NIL;
	if (valSize < 0)
		return EC_INVALID_VAL_SIZE;
	/*
	if (val == NULL)
		return EC_VAL_NIL;
	*/
	return addNVPair(name, strlen(name), val, valSize, valType);
}

// 内部调用
int CPackager::addNVPair(const char *name, size_t nameSize, const void *val, size_t valSize, char valType)
{
	return addNVPair(string(name, nameSize), valSize, val, valType);
}

/**
 * 把所有的name/val对打包，和参数的构造函数对应
 * @return 0:成功; <0: 各种异常
 */
int CPackager::package()
{
	CStrHelper str(m_res);
	
	// name/value长度
	size_t pairs = m_nvs.size();
	for (size_t i=0; i<pairs; ++i)
	{
		str += m_nvs[i].nameSize;
		str += m_nvs[i].valSize;
	}
	
	// 追加meta信息
	str += PACK_VERSION; // 版本号
	str += pairs;        // name/value对个数
	
	return 0;
}

/*--------- 对通过网络传输获得的用CPackager打包的内容进行解析 ---------*/

// meta
struct SPackMeta
{
	int32_t  ver;     // 版本号
	int32_t  nvCount; // name/value对数目
};

// 用于定位name/value
struct SNVPos
{
	size_t nameOffset; // name在m_packBody中的偏移
	size_t nameSize;   // name的字节数
	size_t valOffset;  // value在m_packBody中的偏移
	size_t valSize;    // value的字节数
	size_t typeOffset; // valType在m_packBody中的偏移
};

/**
 * 空构造函数，需要解包的话，可以调用unpackage()
 */
CUnpackager::CUnpackager() : m_valid(false), m_nvPos(NULL)
{
}

/**
 * 构造函数，对指定的包进行解析
 * @param pack      需要解些的包
 * @param packSize  pack的字节数(pack类型为void*需要提供)
 */
CUnpackager::CUnpackager(const std::string &pack) : m_nvPos(NULL)
{
	unpackage(pack.c_str(), pack.length());
}

CUnpackager::CUnpackager(const void *pack, size_t packSize) : m_nvPos(NULL)
{
	unpackage(pack, packSize);
}

/**
 * 析构函数
 */
CUnpackager::~CUnpackager()
{
	if (m_nvPos != NULL)
		delete []m_nvPos;
}

/**
 * 解析一个数据包
 * @param pack      需要解些的包
 * @param packSize  pack的字节数(pack类型为void*需要提供)
 * @return 0:成功; <0:各种异常
 */
int CUnpackager::unpackage(const std::string &pack)
{
	return unpackage(pack.c_str(), pack.length());
}

int CUnpackager::unpackage(const void *pack, size_t packSize)
{
	// 设置初始状态
	m_valid = false;
	m_nameIdx.clear();
	if (m_nvPos != NULL)
	{
		delete []m_nvPos;
		m_nvPos = NULL;
	}
	m_nvCount     = 0;
	m_nextPairIdx = 0;
	
	// 检查参数
	if (pack == NULL)
		return EC_PACK_NIL;
	if (packSize < 0)
		return EC_INVALID_PACK_SIZE;
	if (packSize == 0)
	{
		// 特殊的包
		m_valid = true;
		return 0;
	}
	
	// 包的大小至少包含版本号和name/value对数目
	if (packSize < sizeof(SPackMeta))
		return EC_INVALID_PACK_SIZE;
	
	m_pack      = pack;
	m_packSize  = packSize;
	
	SPackMeta *meta = (SPackMeta*)((const char *)pack + packSize - sizeof(SPackMeta)); // meta在尾部
	
	// 检查版本号
	int32_t ver = ntohl(meta->ver);
	if (ver != PACK_VERSION)
		return EC_UNKNOWN_PACK_VER;
	
	m_nvCount = ntohl(meta->nvCount);
	size_t metaSize = sizeof(SPackMeta) + (m_nvCount * 2) * sizeof(int32_t);
	if (m_nvCount < 0 || packSize < metaSize)
		return EC_INVALID_PACK_SIZE;
	if (m_nvCount == 0)
	{
		// 特殊的包
		m_valid = true;
		return 0;
	}
	int32_t *packNVSize = ((int32_t*)meta) - m_nvCount*2; // name/value长度信息，网络字节顺序
	
	// 申请位置信息空间
	m_nvPos = new SNVPos[m_nvCount];
	if (m_nvPos == NULL)
		return EC_MEM_ALLOC_FAILED;

	m_packBody    = (const char*)pack;
	m_nextPairIdx = 0;
	// 扫描meta，收集name/value的偏移和长度信息(不保存name的信息，有可能长度信息是假的)
	size_t bodySize = 0;
	for (size_t i=0, j=0; i<m_nvCount; ++i)
	{
		m_nvPos[i].nameOffset = bodySize;
		m_nvPos[i].nameSize   = ntohl(packNVSize[j++]);
		bodySize += m_nvPos[i].nameSize;
		bodySize++;                    // '\0' for name
		
		m_nvPos[i].valOffset = bodySize;
		m_nvPos[i].valSize   = ntohl(packNVSize[j++]);
		bodySize += m_nvPos[i].valSize;
		bodySize++;                    // '\0' for value
		
		m_nvPos[i].typeOffset = bodySize;
		bodySize++;                    // valType
	}
	if (metaSize + bodySize != packSize)
		return EC_INVALID_PACK_SIZE;

	//收集所有的name/value的位置信息
	for (size_t i=0; i<m_nvCount; ++i)
	{
		const char *name = m_packBody + m_nvPos[i].nameOffset;
		m_nameIdx[name]  = i; // name -> index in m_nvPos;
	}
	
	// 分析成功
	m_valid = true;
	return 0;
}

/**
 * 获取指定name的值
 * @param name     指定的name
 * @param val      保存name对应的值
 * @param valSize  保存val中的有效字节数(当val的类型为void**时需要提供)
 * @param valType  val的类型(可以抛弃，类型根据需要自行定义，参见CPackager::addNVPair)
 * @return 0:成功; <0:各种异常
 */
int CUnpackager::getValue(const std::string &name, std::string &val, char *valType)
{
	void *v;
	size_t vs;
	int ret = getValue(name.c_str(), &v, &vs, valType);
	if (ret != 0)
		return ret;
	if (vs > 0)
		val.assign((const char*)v, vs);
	else
		val.clear();
	return 0;
}

int CUnpackager::getValue(const char *name, std::string &val, char *valType)
{
	void *v;
	size_t vs;
	int ret = getValue(name, &v, &vs, valType);
	if (ret != 0)
		return ret;
	if (vs > 0)
		val.assign((const char*)v, vs);
	else
		val.clear();
	return 0;
}

int CUnpackager::getValue(const std::string &name, void **val, size_t *valSize, char *valType)
{
	return getValue(name.c_str(), val, valSize, valType);
}

int CUnpackager::getValue(const char *name, void **val, size_t *valSize, char *valType)
{
	if (!m_valid)
		return EC_INVALID_PACKAGE;
	if (name == NULL || name[0] == '\0')
		return EC_NAME_NIL;
	if (val == NULL)
		return EC_VAL_BUFFER_NIL;
	if (valSize == NULL)
		return EC_VAL_SIZE_NIL;

	TNameIndexMap::iterator i = m_nameIdx.find(name);
	if (i == m_nameIdx.end())
		return EC_NAME_NOT_FOUND;
	
	getValueByIdx(i->second, val, valSize, valType);
	return 0;
}

/**
 * 获取下一个name/value对
 * @param name     保存名称
 * @param val      保存value值
 * @param valSize  保存val中的有效字节数(当val的类型为void**时需要提供)
 * @param valType  val的类型(可以抛弃，类型根据需要自行定义，参见CPackager::addNVPair)
 * @return 0:成功; <0:异常或者没有name/value对了
 */	
int CUnpackager::getNextNVPair(std::string &name, std::string &val, char *valType)
{
	char *n;
	void *v;
	size_t vs;
	
	int ret = getNextNVPair(&n, &v, &vs, valType);
	if (ret != 0)
		return ret;
	name.assign(n);
	if (vs > 0)
		val.assign((const char*)v, vs);
	else
		val.clear();
	return 0;
}

int CUnpackager::getNextNVPair(char **name, void **val, size_t *valSize, char *valType)
{
	if (!m_valid)
		return EC_INVALID_PACKAGE;
	
	if (m_nextPairIdx >= m_nvCount)
		return EC_NO_MORE_NV_PAIR;
	
	if (name == NULL)
		return EC_NAME_BUFFER_NIL;
	if (val == NULL)
		return EC_VAL_BUFFER_NIL;
	if (valSize == NULL)
		return EC_VAL_SIZE_NIL;
	
	*name = (char*)m_packBody + m_nvPos[m_nextPairIdx].nameOffset;
	getValueByIdx(m_nextPairIdx++, val, valSize, valType);
	return 0;
}

// 根据下标取value
void CUnpackager::getValueByIdx(size_t i, void **val, size_t *valSize, char *valType)
{
	if (m_nvPos[i].valSize <= 0)
	{
		*val     = NULL;
		*valSize = 0;
	}
	else
	{
		*val     = (void*)(m_packBody + m_nvPos[i].valOffset);
		*valSize = m_nvPos[i].valSize;
	}
	if (valType != NULL)
		*valType = m_packBody[m_nvPos[i].typeOffset]; 
}

}

