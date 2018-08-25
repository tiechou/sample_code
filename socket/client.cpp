#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
using namespace std;

const int BUFSIZE = 128; 
int main()
{
    //create socket
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if(client_socket < 0)
    {
        cout << "create socket error" << endl;
        return -1;
    }

    //set address and bind socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5555);
    server_addr.sin_addr.s_addr = inet_addr("10.232.41.135");

    if(connect(client_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
    {
        cout << "connect server error" << endl;
        close(client_socket);
        return -1;
    }

    char buf[BUFSIZE];
    if(recv(client_socket,buf,BUFSIZE,0) < 0)
    {
        close(client_socket);
        return -1;
    }
    else
    {
        cout << buf << endl;
        close(client_socket);
        return 0;
    }
}

