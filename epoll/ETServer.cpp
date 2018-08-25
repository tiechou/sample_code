#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <errno.h> 
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAXLINE 1024 
#define BUFSIZE 11 
#define LISTENQ 10
#define SERV_PORT 5555 

using namespace std; 

void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }   
}

int main()
{
    int i, maxi, listenfd, connfd, sockfd,epfd,nfds;
    ssize_t n;
    char line[MAXLINE];
    memset(line,0,MAXLINE);
    socklen_t clilen;
    
    //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
    struct epoll_event ev,events[20];
    
    //生成用于处理accept的epoll专用的文件描述符
    epfd = epoll_create(256);

    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    //把socket设置为非阻塞方式
    setnonblocking(listenfd);
    //设置与要处理的事件相关的文件描述符
    ev.data.fd = listenfd;
    //设置要处理的事件类型
    ev.events=EPOLLIN|EPOLLET;
    //注册epoll事件
    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERV_PORT);
    bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, LISTENQ);
    maxi = 0;
    while(1){
        
        //等待epoll事件的发生
        nfds=epoll_wait(epfd,events,20,500);
        
        //处理所发生的所有事件    
        if(nfds > 0){printf("ndfs : %d\n" , nfds);	}//sleep(10);}
        for(i=0; i<nfds; ++i)
        {
            if(events[i].data.fd==listenfd)
            {

                connfd = accept(listenfd,(sockaddr *)&clientaddr, &clilen);
                if(connfd < 0)
                {
                    if(errno == EAGAIN){
                    } else {
                        perror("connect error");
                        exit(1);
                    }
                }
                setnonblocking(connfd);

                char *str = inet_ntoa(clientaddr.sin_addr);
                //设置用于读操作的文件描述符
                ev.data.fd=connfd;
                //设置用于注测的读操作事件
                //ev.events=EPOLLOUT|EPOLLIN|EPOLLET;
                ev.events=EPOLLIN|EPOLLET;
                //注册ev
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
                //统计本次accept了多少个连接。
            }
            else if(events[i].events&EPOLLIN)
            {
                memset(line,0,MAXLINE);
                int count = 0;
                if ( (sockfd = events[i].data.fd) < 0) continue;
                char buf[BUFSIZE];
                while(1){	
                    cout << "read fd=" << sockfd << ", count= "<< ++count <<endl;
                    memset(buf,0,BUFSIZE);
                    if ( (n = read(sockfd, buf, BUFSIZE-1)) < 0) {
                        if (errno == EAGAIN) {
                            break;
                        } else{
                            perror("recv error!!");
                            close(sockfd);
                            epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&ev);
                            exit(1);
                        }
                    } 
                    strcat(line,buf);  
                    if(n < BUFSIZE-1){
                        break;
                    }
                }
                cout<<"recv :"<<line<<endl;
                //设置用于写操作的文件描述符
                ev.data.fd=sockfd;
                //设置用于注测的写操作事件
                ev.events=EPOLLOUT|EPOLLET;
                //ev.events=EPOLLIN|EPOLLET;
                //修改sockfd上要处理的事件为EPOLLOUT
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
            else if(events[i].events&EPOLLOUT)
            {  
                cout << "send fd" << sockfd << endl;
                sockfd = events[i].data.fd;
                char * plin = line;
                size_t len = sizeof(plin);
                while(1){
                    ssize_t tmp =write(sockfd, plin, len);
                    if(tmp < 0)
                    {
                        if(errno == EAGAIN){
                            /*缓冲区满了*/
                            break;
                        } else{
                            perror("send  error");
                            exit(1);
                        }
                    }
                    else if(tmp < len){
                        len -= tmp;
                        plin += tmp;
                    }
                    else if(tmp == len) break;
                }
                cout << "send :" << line << endl;
                //close(sockfd);
                //events[i].data.fd = -1;
                ev.data.fd=sockfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
        }// for
    }// while(1)
    return 0;
}
