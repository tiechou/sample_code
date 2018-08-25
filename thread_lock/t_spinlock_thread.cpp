#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/time.h>
#include <sched.h>
#include <linux/unistd.h>
#include <list>
#include "TimeHelper.h"

#define MAX_ARRAY_NUM 10000000

using namespace std;

union AlignInt32
{
    uint32_t _member;
    char _align[64]; // for false sharing for multi-core

};

struct StatItem
{
    uint32_t    _times;
    uint32_t    _id;
};
union AlignStat
{
    StatItem _item;
    char _align[64]; // for false sharing for multi-core
};

AlignInt32 g_Array[10000000];
volatile uint32_t g_Index=0;

#ifdef USE_SPINLOCK
pthread_spinlock_t spinlock;
#else
pthread_mutex_t mutex;
#endif

pid_t gettid() { return syscall( __NR_gettid ); }

void *consumer(void *arg)
{
    AlignStat* pItem=(AlignStat*)arg;
    while (1)
    {
#ifdef USE_SPINLOCK
        pthread_spin_lock(&spinlock);
#else
        pthread_mutex_lock(&mutex);
#endif

        if (g_Index>=MAX_ARRAY_NUM)
        {
#ifdef USE_SPINLOCK
            pthread_spin_unlock(&spinlock);
#else
            pthread_mutex_unlock(&mutex);
#endif
            break;
        }
        ++(pItem->_item._times);
        g_Array[g_Index]._member=g_Index;
        ++g_Index;

#ifdef USE_SPINLOCK
        pthread_spin_unlock(&spinlock);
#else
        pthread_mutex_unlock(&mutex);
#endif

    }

    return NULL;
}

int main(int argc, char *argv[])
{

    uint64_t t1,t2;
    uint64_t nTimeSum=0;
    uint32_t nThreadNum=0;

#ifdef USE_SPINLOCK
    pthread_spin_init(&spinlock, 0);
    fprintf(stderr,"case for spinlock: ");
#else
    pthread_mutex_init(&mutex, NULL);
    fprintf(stderr,"case for mutex: ");
#endif

    int32_t nCpuNum = (int)sysconf( _SC_NPROCESSORS_ONLN )*2;
    fprintf(stderr,"cpu_num=%d\n",nCpuNum/2);
    for(int32_t j=1; j< nCpuNum; j++)
    {
        nTimeSum=0;
        nThreadNum=j;
        AlignStat *pStatArray= new AlignStat[nThreadNum];
        memset(pStatArray,0x0,nThreadNum*sizeof(AlignStat));

        for(uint32_t nLoop=10; nLoop> 0 ; nLoop--)
        {
            g_Index=0;
            pthread_t * pThreadArray=new pthread_t[nThreadNum];
            // Measuring time before starting the threads...
            t1=TimeHelper::nowTime();
            for(uint32_t i=0; i <nThreadNum; i++)
            {
                pStatArray[i]._item._id=i;
                if ( pthread_create(&pThreadArray[i], NULL, consumer, (void *)(&pStatArray[i]) ))
                {
                    perror( "error: pthread_create" );
                    nThreadNum = i;
                    break;
                }
            }
            for(uint32_t i=0; i <nThreadNum; i++)
            {
                pthread_join(pThreadArray[i], NULL);
            }
            // Measuring time after threads finished...
            t2=TimeHelper::nowTime();
            nTimeSum+=t2-t1;
            delete [] pThreadArray;
        }

        fprintf(stderr,"RepeatTimes=%d, ThreadNum=%d, UsedTime=%.6lf s\n",10, nThreadNum,(double(nTimeSum))/1000000);
        for(uint32_t i=0; i <nThreadNum; i++)
        {
            fprintf(stderr,"thread_id=%u\t times=%u\n",pStatArray[i]._item._id,pStatArray[i]._item._times);
        }
        delete [] pStatArray;
      }

#ifdef USE_SPINLOCK
    pthread_spin_destroy(&spinlock);
#else
    pthread_mutex_destroy(&mutex);
#endif

    return 0;
}
