/**
 * ����ͽ�����ߣ��Ѷ��name/value�Ժϳ�һ���� �� �Ѱ�������name/value�ԣ��������������紫��
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

// ���ִ�����
enum EErrCode
{
	EC_NO_ERROR         = 0,
	EC_DUPLICATE_NAME   = 1, // �ظ�����
	EC_NAME_NIL         = 2, // NAMEָ��ĵ�ַΪNULL
	EC_VAL_NIL          = 3, // VALUEָ��ĵ�ַΪNULL
	EC_INVALID_VAL_SIZE = 4, // value�ֽ����Ƿ�
	EC_PACK_NIL         = 5, // ��Ҫ�����packageû������
	EC_INVALID_PACK_SIZE= 6, // ��Ҫ������ֽ����Ƿ�
	EC_INVALID_PACKAGE  = 7, // �Ƿ���package
	EC_UNKNOWN_PACK_VER = 8, // δ֪��package�汾
	EC_NAME_BUFFER_NIL  = 9, // ���name�Ļ�����ΪNULL
	EC_VAL_BUFFER_NIL   = 10,// ���value�Ļ�����ΪNULL
	EC_VAL_SIZE_NIL     = 11,// ���value���ȵĵ�ַΪNULL
	EC_NAME_NOT_FOUND   = 12,// û��ָ����name��Ӧ��value
	EC_MEM_ALLOC_FAILED = 13,// �����ڴ�ʧ��
	EC_NO_MORE_NV_PAIR  = 14,// û��name/value��
	EC_PACK_NOT_MATCHED = 15,// �����˲����׵�package����
	
	TOAL_EC
};

struct SNV;                         // name/value�ԣ��ڲ�ʹ��
typedef std::vector<SNV>  TNVPairs; // name/value�Լ���

// �����ִ�Сд�Ƚ��࣬����TNameIndexMapʹ��
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
typedef std::map<const char*, size_t, SStrCaseCmp> TNameIndexMap; // name => �±�, name�����ִ�Сд
typedef std::set<std::string, SStrCaseCmp>         TNameSet;

/**
 * ������ߣ��Ѷ��name/value�Ժϳ�һ�������������������紫��
 */
class CPackager
{
public:
	/**
	 * ����Ŵ������Ĺ��캯��
	 * @param res  ���ڴ�Ŵ�����
	 * [ע]��ʹ�ú�����addNVPairʱ�����ͷ������õ�name��value�ռ�
	 *     �������Լ���Ҫ�޸�resֵ
	 *     ͨ���޲�����package()��ô�����
	 */ 
	CPackager(std::string &res);
	
	/** �������� */
	virtual ~CPackager();
	
	/**
 	 * ���һ��name/value��
	 * @param name    name����
	 * @param valLen  val���ֽ���(val����Ϊvoid*ʱ��Ҫ�ṩ)
	 * @param val     valueֵ�����value��һ�����ӵĽṹ�壬�����������л�
	 * @param valType val����������(ȱʡΪ0����ʾδ֪������ֵ������Ҫ����ָ��)
	 * @return  0:�ɹ���<0:�����쳣
	 * [ע]1. ������޲ε�CPackager����û�е���packageǰ����Ҫ�ͷ�name��valָ�������
	 *     2. ���ʹ��void*����val��valSize����������valǰ
 	 */
	int addNVPair(const std::string &name, const std::string &val, char valType=0);
	int addNVPair(const std::string &name, const char *val, char valType=0);
	int addNVPair(const std::string &name, size_t valSize, const void *val, char valType=0);
	int addNVPair(const char *name, const std::string &val, char valType=0);
	int addNVPair(const char *name, const char *val, char valType=0);
	int addNVPair(const char *name, size_t valSize, const void *val, char valType=0);
	
	/**
	 * �����е�name/val�Դ�����Ͳ����Ĺ��캯����Ӧ
	 * @return 0:�ɹ�; <0: �����쳣
 	 */
	int package();
	
	/**
	 * ������״̬������֧���µ�addNVPair
 	 */
	void clear();

protected:
	std::string       &m_res;     // ����������������Ľ��
	
	std::vector<SNV>   m_nvs;     // ��������ͨ��addNVPair�����name/value��
	TNameSet           m_nameSet; // ��m_res!=NULLʱʹ�ã�����name�ظ�
	
private:
	int addNVPair(const char *name, size_t nameSize, const void *val, size_t valSize, char valType);
};

struct SNVPos; // ���ڶ�λname/value�Ľṹ

/**
 * ��ͨ�����紫���õ���CPackager��������ݽ��н���
 */
class CUnpackager
{
public:
	/**
	 * �չ��캯������Ҫ����Ļ������Ե���unpackage()
	 */
	CUnpackager();
	/**
	 * ���캯������ָ���İ����н���
	 * @param pack      ��Ҫ��Щ�İ�
	 * @param packSize  pack���ֽ���(pack����Ϊvoid*��Ҫ�ṩ)
	 */
	CUnpackager(const std::string &pack);
	CUnpackager(const void *pack, size_t packSize);

	/** �������� */
	virtual ~CUnpackager();
	
	/**
	 * ����һ�����ݰ�
	 * @param pack      ��Ҫ��Щ�İ�
	 * @param packSize  pack���ֽ���(pack����Ϊvoid*��Ҫ�ṩ)
	 * @return 0:�ɹ�; <0:�����쳣
	 */
	int unpackage(const std::string &pack);
	int unpackage(const void *pack, size_t packSize);
	
	/**
	 * ��ȡָ��name��ֵ
	 * @param name     ָ����name
	 * @param val      ����name��Ӧ��ֵ
	 * @param valSize  ����val�е���Ч�ֽ���(��val������Ϊvoid**ʱ��Ҫ�ṩ)
	 * @param valType  val������(�������������͸�����Ҫ���ж��壬�μ�CPackager::addNVPair)
	 * @return 0:�ɹ�; <0:�����쳣
	 */
	int getValue(const std::string &name, std::string &val, char *valType=NULL);
	int getValue(const char *name, std::string &val, char *valType=NULL);
	int getValue(const std::string &name, void **val, size_t *valSize, char *valType=NULL);
	int getValue(const char *name, void **val, size_t *valSize, char *valType=NULL);

	/**
	 * ��ȡ��һ��name/value��
	 * @param name     ��������
	 * @param val      ����valueֵ
	 * @param valSize  ����val�е���Ч�ֽ���(��val������Ϊvoid**ʱ��Ҫ�ṩ)
	 * @param valType  val������(�������������͸�����Ҫ���ж��壬�μ�CPackager::addNVPair)
	 * @return 0:�ɹ�; <0:�쳣����û��name/value����
 	 */	
	int getNextNVPair(std::string &name, std::string &val, char *valType=NULL);
	int getNextNVPair(char **name, void **val, size_t *valSize, char *valType=NULL);

protected:
	bool           m_valid;       // ���Ƿ�Ϸ�?
	const void    *m_pack;        // ������Ҫ�����İ�
	size_t         m_packSize;    // pack���ֽ���
	const char    *m_packBody;    // pack�а������ʼ��ַ
	
	size_t         m_nvCount;     // name/value����
	SNVPos        *m_nvPos;       // ���name/value��m_packBody�е�λ����Ϣ
	size_t         m_nextPairIdx; // ��¼����getNextNVPairʱʹ�õ�m_pack���±�
	
	TNameIndexMap  m_nameIdx;     // name => index in m_nvPos�����ٲ���

private:
	void getValueByIdx(size_t i, void **val, size_t *valSize, char *valType); // �����±�ȡvalue
};

}

#endif

