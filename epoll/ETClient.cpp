#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>

#define PORT 5555 
#define MAXLINE 51000 
using namespace std;

int main(int argc, char* argv[], char* evp[])
{
    int fd;
    int on = 1;

    struct sockaddr_in seraddr;
    memset(&seraddr, 0, sizeof(seraddr));
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "客户端创建套接字出错了" << std::endl;
        return -1;
    }

    //如果用于多次测试，那么打开下面debug选项
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    seraddr.sin_port = htons((short)(PORT));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //你也可以采用无阻塞连接，不过需要对连接的错误结果进行分析处理
    if(connect(fd, (struct sockaddr*)&seraddr, sizeof(seraddr)) < 0)
    {
        std::cout << "连接错误了" << std::endl;
        return -1;
    }

    char buffer[512];
    char* wbuffer = "The Author@: magicminglee@Hotmail.com";
    memset(buffer, 0 , sizeof(buffer));
    strncpy(buffer, wbuffer, strlen(wbuffer));

    send(fd, buffer, strlen(buffer) , 0);
    std::cout << "send1 = "<<buffer << std::endl;

    memset(buffer, 0 , sizeof(buffer));
    int n=0;
    char line[MAXLINE];
    int flag=1;
    memset(line, 0 , sizeof(line));
    cout << "recv fd" << fd << endl;
    if ( (n = read(fd, line, MAXLINE)) < 0) {
        if (errno == EAGAIN) {
        } else{
            perror("recv error!!");
            exit(1);
        }
    } 
    cout<<"recv :"<<line<<endl;

    bcopy("The Author", buffer, sizeof("The Author"));
    send(fd, buffer, strlen(buffer) , 0);
    std::cout << buffer << std::endl;

    n=read(fd, buffer, sizeof(buffer));
    std::cout << buffer << std::endl;
    
    shutdown(fd, SHUT_RDWR);
    close(fd);
    exit(0);
}
