#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
using namespace std;

void* thread_func(void* arg);

int main()
{
    //create socket
    int listen_sock = socket(AF_INET,SOCK_STREAM,0);
    if(listen_sock < 0)
    {
        cout << "create socket error" << endl;
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(5555);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listen_sock,(struct sockaddr*)&address,sizeof(address)) < 0)
    {
        cout << "bind socket error" << endl;
        close(listen_sock);
        return -1;
    }

    listen(listen_sock,10);

    while(true)
    {
        void *ret;
        int sock = accept(listen_sock,NULL,NULL);
        pthread_t tid;
        pthread_create(&tid,NULL,thread_func,&sock);
        // pthread_join(tid,&ret);
        sleep(1);

    }

    close(listen_sock);
    return 0;
}

// thread function
void* thread_func(void *arg)
{
    int sock = *((int*)arg);
    cout << pthread_self() <<" send data" << endl;
    send(sock,"hello world",strlen("hello world")+1,0);
    close(sock);

    return (void*)0;
}

