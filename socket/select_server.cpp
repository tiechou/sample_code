#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/ioctl.h>
using namespace std;

int main()
{
    int server_sock = socket(AF_INET,SOCK_STREAM,0);
    if(server_sock < 0)
    {
        cout << "create socket error" << endl;
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5555);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_sock,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
    {
        cout << "bind socket error" << endl;
        close(server_sock);
        return -1;
    }

    listen(server_sock,10);

    fd_set read_set;
    fd_set test_set;
    FD_ZERO(&read_set);
    FD_SET(server_sock,&read_set);

    struct timeval tm;
    tm.tv_sec = 5;
    tm.tv_usec = 500;


    while(true)
    {
        int nread = 0;
        test_set = read_set;
        int ret = select(FD_SETSIZE,&test_set,NULL,NULL,&tm);

        if(ret < 0)
        {
            cout << "select error" << endl;
            close(server_sock);
            return -1;
        }
        else if(ret == 0)
        {
            //cout << "waitout" << endl;
            continue;
        }
        else
        {
            for(int fd=0; fd<FD_SETSIZE; ++fd)
            {
                if(FD_ISSET(fd,&test_set))
                {
                    //如果有新的连接到达
                    if(fd == server_sock)
                    {
                        int sock = accept(server_sock,NULL,NULL);
                        FD_SET(sock,&read_set);
                    }
                    else
                    {
                        ioctl(fd,FIONREAD,&nread);

                        if(nread == 0) //客户端已经关闭
                        {
                            close(fd);
                            FD_CLR(fd,&read_set);
                            cout << "client has removed " << fd << endl;
                        }
                        else
                        {
                            char buf[128];
                            recv(fd,buf,128,0);
                            cout << buf << endl;
                            send(fd,buf,strlen(buf)+1,0);
                        }
                    }

                }

            }

        }

    }
}

