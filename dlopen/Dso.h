
#ifndef  __DSO_DSO_H_
#define  __DSO_DSO_H_
#include  <stdint.h>
#include  <string>
#include  "DsoBase.h"

namespace  name_1
{
class  Dso:public  DsoBase
{
public:
    Dso()  ;
    virtual  ~Dso()  ;
    virtual  void  toString();
private:
    static  pthread_mutex_t  _mutex;
    static  char  *  _buff;     //  静态成员变量
    const   uint32_t  MAX_LEN;
};
}
#endif
