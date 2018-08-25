#include  <stdio.h>
#include  "Dso.h"

namespace  name_1
{

        pthread_mutex_t  Dso::_mutex  =  PTHREAD_MUTEX_INITIALIZER;
        char  *  Dso::_buff=NULL;    //  静态成员变量
        
        Dso::Dso():MAX_LEN(10*1024*1024)
        {
            printf("Dso  constructor\n");
        }
        Dso::~Dso()
        {
            printf("Dso  destructor\n");
        }
        
        void  Dso::toString()
        {
            pthread_mutex_lock(&_mutex);
            if(_buff  ==  NULL)
            {//第1次使用是申请分配10M内存
                _buff=(char*)malloc(MAX_LEN);
                memset(_buff,0x0,MAX_LEN);
                printf("_buff=%p\n",_buff);
            }
            pthread_mutex_unlock(&_mutex);
        }

}

extern  "C"  name_1::DsoBase  *  CreateFun()
{
    return   new  name_1::Dso();
}
extern  "C"  void  DestroyFun(name_1::DsoBase  *  obj)
{
    delete  obj;
}
