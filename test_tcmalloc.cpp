/*
 * ./configure --prefix=/home/henshao/googleperf_install && make -j 8 && make install
 * 编译并安装gperftools-2.0
 * ./configure --prefix=/home/henshao/googleperf_install CPPFLAGS=-I/home/henshao/googleperf_install/include/ LDFLAGS=-L/home/henshao/googleperf_install/lib/ && make -j 8 && make install
 *
 * 编译
 * g++ test_tcmalloc.cpp -o test_tcmalloc -lpthread
 *
 * 使用tcmalloc
 * env LD_PRELOAD="/home/henshao/googleperf_install/lib/libtcmalloc.so" MALLOCSTATS=2 HEAPPROFILE="./perf_log/perf_leak.log" ./test_tcmalloc
 *
 * 通过top命令可以看到，程序的物理内存在不断增加。
 * $top -p `pgrep -f test_tcmalloc`
 *
 * 通过heap profiler可以看出，程序是没有内存泄露的。
 * pprof --text google_perf_test perf_log/perf_leak.log.2135.heap
 *
 */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
 
pthread_mutex_t mutex;
char * ptr = NULL;
bool need_free = false;
bool quit = false;
 
void* do_malloc(void* arg)
{
    printf("do_malloc thread running...\n");
 
    while (true) {
        //usleep(1);
        if (quit) {
            printf("do_malloc thread quit\n");
            return NULL;
        }
 
        pthread_mutex_lock(&mutex);
        if (need_free) {
            pthread_mutex_unlock(&mutex);
            continue;
        }else{
            int size = (5120*random()) % 1024000;
            ptr = (char*)malloc(size);
            memset(ptr, 'a', size);
 
            need_free = true;
            //printf("malloc %d bytes\n", size);
            pthread_mutex_unlock(&mutex);
        }
    }
}
 
void* do_free(void* arg)
{
    printf("do_free thread running...\n");
 
    while (true) 
    {
        //usleep(1);
        if (quit) {
            printf("do_free thread quit\n");
            return NULL;
        }
 
        pthread_mutex_lock(&mutex);
        if (ptr) {
            free(ptr);
            ptr=NULL;
            need_free = false;
            //printf("free\n");
        }
        pthread_mutex_unlock(&mutex);
    }
}
 
void sigroutine(int dunno) {
    printf("quit\n");
    quit = true;
 
    return;
}
 
int main(int argc, const char *argv[])
{
    signal(SIGINT, sigroutine);
 
    pthread_t malloc_pid, free_pid;
 
    srand((unsigned)time(0));
    pthread_mutex_init(&mutex, NULL);
 
    pthread_create(&malloc_pid, NULL, do_malloc, NULL);
    pthread_create(&free_pid, NULL, do_free, NULL);
 
    pthread_join(malloc_pid, NULL);
    pthread_join(free_pid, NULL);
 
    return 0;
}
