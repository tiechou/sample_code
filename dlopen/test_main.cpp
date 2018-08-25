#include <stdio.h>
#include <dlfcn.h>
#include "pthread.h"
#include "Dso.h"

typedef name_1::DsoBase * (*CreateFunT)();
typedef void (*DestroyFunT)(name_1::DsoBase * p);

int main(int argc , char ** argv)
{
    // 模拟 ha3 插件更新过程
    for(int i=0; i <50; i ++ )
    {
        //1. dl_open libdso_a.so
        void * dl_a=dlopen("./libdso_a.so",RTLD_NOW);
        if(!dl_a ) {
            printf("dlopen return %s", dlerror());
            return -1;
        }
        CreateFunT createFun_a= (CreateFunT)dlsym(dl_a , "CreateFun");
        DestroyFunT destoryFunc_a = (DestroyFunT)dlsym(dl_a , "DestroyFun");
        name_1::DsoBase* obj_a=createFun_a();
        obj_a->toString();

        //2. dl_open libdso_b.so
        void * dl_b=dlopen("./libdso_b.so",RTLD_NOW);
        if(!dl_b ) {
            printf("dlopen return %s", dlerror());
            return -1;
        }
        CreateFunT createFun_b= (CreateFunT)dlsym(dl_b , "CreateFun");
        DestroyFunT destoryFunc_b = (DestroyFunT)dlsym(dl_b , "DestroyFun");
        name_1::DsoBase* obj_b=createFun_b();
        obj_b->toString();
        sleep(5);
       //3. dl_close libdso_a.so
        destoryFunc_a(obj_a);
        dlclose(dl_a);
        sleep(5);
       //4. 使用 obj_b
        obj_b->toString();

       //5. dl_close libdso_a.so
        destoryFunc_b(obj_b);
        dlclose(dl_b);
        sleep(20);
    }
    return 0;
}
