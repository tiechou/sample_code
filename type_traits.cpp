// Traits_example1.cpp
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// 枚举类型定义
typedef enum FieldType
{
    FIELDTYPE_UNDEFINED = 0,
    FIELDTYPE_INT8,
    FIELDTYPE_INT32, //为简化问题，只列举两种
} EFieldType;

// 利用Traits编程技巧
template<FieldType pt>
struct VariableTypeTraits {
    typedef void SyntaxType;
};
template<>
struct VariableTypeTraits<FIELDTYPE_INT8> {
    typedef int8_t SyntaxType;
};
template<>
struct VariableTypeTraits<FIELDTYPE_INT32> {
    typedef int32_t SyntaxType;
};
// 测试demo函数
void func(int8_t var)
{
    fprintf(stdout, "type: int8_t; value: %d", var);
}
void func(int32_t var)
{
    fprintf(stdout, "type: int32_t; value: %d", var);
}

int main()
{
    typedef VariableTypeTraits<FIELDTYPE_INT8>::SyntaxType int8_type;
    int8_type int8_a = 1;
    func(int8_a);
    typedef VariableTypeTraits<FIELDTYPE_INT32>::SyntaxType int32_type;
    int32_type int32_b = 2;
    func(int32_b);
    return 0;
}
