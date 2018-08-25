
#ifndef __DSO_BASE_H_
#define __DSO_BASE_H_
namespace name_1
{
class DsoBase
{
public:
    DsoBase(){};
    virtual ~DsoBase(){};
    virtual void toString()=0;
};
}
#endif
