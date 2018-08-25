#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
using namespace std;

int main()
{
    //create socket handle
    int listen_sock = socket(AF_INET,SOCK_STREAM,0);
    if(listen_sock < 0)
    {
        cout << "create socket error" << endl;
        return -1;
    }

    //bind socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5555);
    server_addr.sin_addr.s_addr = inet_addr("10.232.41.135");
    if(bind(listen_sock,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
    {
        cout << "bind socket error" << endl;
        close(listen_sock);
        return -1;
    }

    listen(listen_sock,5);
    //wait for connection
    while(true)
    {
        int sock = accept(listen_sock,NULL,NULL);
        send(sock,"hello world",strlen("hello world")+1,0);
        close(sock);
    }

    close(listen_sock);
    return 0;
}

