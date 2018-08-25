/**
 * ����ͽ�����ߣ��Ѷ��name/value�Ժϳ�һ���� �� �Ѱ�������name/value�ԣ��������������紫��
 * ���ĸ�ʽ��
 *   <����><meta��Ϣ>
 * ����<meta��Ϣ>��ʽ
 *   <n1����><v1����><n2����><v2����>...<n_n����><v_n����><�汾><nv����Ŀ>
 *   [��������ȫ��Ϊ4�ֽ����ͣ������ֽ�˳��]
 * <����>��ʽ
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

// ��������
const int32_t PACK_VERSION  = 0x00010000; // �汾��, 4�ֽڣ���2�ֽ�Ϊ���汾�ţ���2�ֽ�Ϊ�ΰ汾��
const char    PACK_DELI     = '\0';       // ���ڲ�ʹ�õķָ�������Ҫ��Ϊ��Ӧ�ô�����

// name/value����Ϣ
struct SNV
{
	size_t      nameSize;
	size_t      valSize;

	SNV(size_t nameSize, size_t valSize) :
		nameSize(nameSize), valSize(valSize) {}
};

// �ַ��������֧࣬��׷��int �� 2��������
class CStrHelper
{
public:
	CStrHelper(string& s) : m_str(s) {}

	inline CStrHelper& operator+=(const string &s)  { m_str += s; return *this; }
	inline CStrHelper& operator+=(const char* s)    { m_str += s; return *this; }
	inline CStrHelper& operator+=(const char c)     { m_str += c; return *this; }
	inline CStrHelper& operator+=(const int32_t i)
	{
		// ת���������ֽ�˳��׷�ӵ�string
		int32_t ni = htonl(i);
		m_str.append((const char*)&ni, sizeof(ni));
		return *this;
	}
	
	// 64λ�ض�Ϊ32λ
	inline CStrHelper& operator+=(const size_t i)   { return this->operator+=((const int32_t)i); } 
	
	inline CStrHelper& append(const char *v, size_t size) { m_str.append(v, size); return *this; }
	inline CStrHelper& append(const void *v, size_t size) { m_str.append((const char*)v, size); return *this; }

	inline string& operator*() { return m_str; }
	inline void clear() { m_str.clear(); }
	inline void reserve(size_t num) { m_str.reserve(num); }
private:
	string& m_str;
};

/*------------  ������ߣ��Ѷ��name/value�Ժϳ�һ�������������������紫�� -----------*/

/**
 * ����Ŵ������Ĺ��캯��
 * @param res  ���ڴ�Ŵ�����
 */
CPackager::CPackager(std::string &res) : m_res(res)
{
	m_res.clear();
}

/** �������� */
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
 * ���һ��name/value��
 * @param name    name����
 * @param valLen  val���ֽ���(val����Ϊvoid*ʱ��Ҫ�ṩ)
 * @param val     valueֵ�����value��һ�����ӵĽṹ�壬�����������л�
 * @param valType val����������(ȱʡΪ0����ʾδ֪������ֵ������Ҫ����ָ��)
 * @return  0:�ɹ���<0:�����쳣
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
	
	// ����Ƿ�����
	TNameSet::iterator i = m_nameSet.find(name);
	if (i != m_nameSet.end())
		return EC_DUPLICATE_NAME;
	
	// ֱ�Ӵ���������
	CStrHelper str(m_res);
	str += name;
	str += '\0';
	str.append((const char*)val, valSize);
	str += '\0';
	str += valType;
	
	// ����name/value�ĵ�ַ��Ϣ������
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

// �ڲ�����
int CPackager::addNVPair(const char *name, size_t nameSize, const void *val, size_t valSize, char valType)
{
	return addNVPair(string(name, nameSize), valSize, val, valType);
}

/**
 * �����е�name/val�Դ�����Ͳ����Ĺ��캯����Ӧ
 * @return 0:�ɹ�; <0: �����쳣
 */
int CPackager::package()
{
	CStrHelper str(m_res);
	
	// name/value����
	size_t pairs = m_nvs.size();
	for (size_t i=0; i<pairs; ++i)
	{
		str += m_nvs[i].nameSize;
		str += m_nvs[i].valSize;
	}
	
	// ׷��meta��Ϣ
	str += PACK_VERSION; // �汾��
	str += pairs;        // name/value�Ը���
	
	return 0;
}

/*--------- ��ͨ�����紫���õ���CPackager��������ݽ��н��� ---------*/

// meta
struct SPackMeta
{
	int32_t  ver;     // �汾��
	int32_t  nvCount; // name/value����Ŀ
};

// ���ڶ�λname/value
struct SNVPos
{
	size_t nameOffset; // name��m_packBody�е�ƫ��
	size_t nameSize;   // name���ֽ���
	size_t valOffset;  // value��m_packBody�е�ƫ��
	size_t valSize;    // value���ֽ���
	size_t typeOffset; // valType��m_packBody�е�ƫ��
};

/**
 * �չ��캯������Ҫ����Ļ������Ե���unpackage()
 */
CUnpackager::CUnpackager() : m_valid(false), m_nvPos(NULL)
{
}

/**
 * ���캯������ָ���İ����н���
 * @param pack      ��Ҫ��Щ�İ�
 * @param packSize  pack���ֽ���(pack����Ϊvoid*��Ҫ�ṩ)
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
 * ��������
 */
CUnpackager::~CUnpackager()
{
	if (m_nvPos != NULL)
		delete []m_nvPos;
}

/**
 * ����һ�����ݰ�
 * @param pack      ��Ҫ��Щ�İ�
 * @param packSize  pack���ֽ���(pack����Ϊvoid*��Ҫ�ṩ)
 * @return 0:�ɹ�; <0:�����쳣
 */
int CUnpackager::unpackage(const std::string &pack)
{
	return unpackage(pack.c_str(), pack.length());
}

int CUnpackager::unpackage(const void *pack, size_t packSize)
{
	// ���ó�ʼ״̬
	m_valid = false;
	m_nameIdx.clear();
	if (m_nvPos != NULL)
	{
		delete []m_nvPos;
		m_nvPos = NULL;
	}
	m_nvCount     = 0;
	m_nextPairIdx = 0;
	
	// ������
	if (pack == NULL)
		return EC_PACK_NIL;
	if (packSize < 0)
		return EC_INVALID_PACK_SIZE;
	if (packSize == 0)
	{
		// ����İ�
		m_valid = true;
		return 0;
	}
	
	// ���Ĵ�С���ٰ����汾�ź�name/value����Ŀ
	if (packSize < sizeof(SPackMeta))
		return EC_INVALID_PACK_SIZE;
	
	m_pack      = pack;
	m_packSize  = packSize;
	
	SPackMeta *meta = (SPackMeta*)((const char *)pack + packSize - sizeof(SPackMeta)); // meta��β��
	
	// ���汾��
	int32_t ver = ntohl(meta->ver);
	if (ver != PACK_VERSION)
		return EC_UNKNOWN_PACK_VER;
	
	m_nvCount = ntohl(meta->nvCount);
	size_t metaSize = sizeof(SPackMeta) + (m_nvCount * 2) * sizeof(int32_t);
	if (m_nvCount < 0 || packSize < metaSize)
		return EC_INVALID_PACK_SIZE;
	if (m_nvCount == 0)
	{
		// ����İ�
		m_valid = true;
		return 0;
	}
	int32_t *packNVSize = ((int32_t*)meta) - m_nvCount*2; // name/value������Ϣ�������ֽ�˳��
	
	// ����λ����Ϣ�ռ�
	m_nvPos = new SNVPos[m_nvCount];
	if (m_nvPos == NULL)
		return EC_MEM_ALLOC_FAILED;

	m_packBody    = (const char*)pack;
	m_nextPairIdx = 0;
	// ɨ��meta���ռ�name/value��ƫ�ƺͳ�����Ϣ(������name����Ϣ���п��ܳ�����Ϣ�Ǽٵ�)
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

	//�ռ����е�name/value��λ����Ϣ
	for (size_t i=0; i<m_nvCount; ++i)
	{
		const char *name = m_packBody + m_nvPos[i].nameOffset;
		m_nameIdx[name]  = i; // name -> index in m_nvPos;
	}
	
	// �����ɹ�
	m_valid = true;
	return 0;
}

/**
 * ��ȡָ��name��ֵ
 * @param name     ָ����name
 * @param val      ����name��Ӧ��ֵ
 * @param valSize  ����val�е���Ч�ֽ���(��val������Ϊvoid**ʱ��Ҫ�ṩ)
 * @param valType  val������(�������������͸�����Ҫ���ж��壬�μ�CPackager::addNVPair)
 * @return 0:�ɹ�; <0:�����쳣
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
 * ��ȡ��һ��name/value��
 * @param name     ��������
 * @param val      ����valueֵ
 * @param valSize  ����val�е���Ч�ֽ���(��val������Ϊvoid**ʱ��Ҫ�ṩ)
 * @param valType  val������(�������������͸�����Ҫ���ж��壬�μ�CPackager::addNVPair)
 * @return 0:�ɹ�; <0:�쳣����û��name/value����
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

// �����±�ȡvalue
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

