//
// Created by Administrator on 2019/6/27 0027.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>


#define maxlisten	10
#define MAXEPOLL	10000

//设置非阻塞模式
int setnonblocking( int fd )
{
    int ret = fcntl(fd,  F_SETFL, O_NONBLOCK );
    if(ret < 0)
        printf("Setnonblocking error : %d\n", errno);
    return 0;
}

void readUntilEmpty(int conn) {
    int r_len;
    char buf[1024];
    do {
        r_len = recv(conn, buf, 1024, 0);
    } while (r_len > 0);
}

int dataRead(int fd, int epoll_fd, char* r_buf, int size) {
    if (r_buf == nullptr) {
        readUntilEmpty(fd);
        printf("buf is nullptr");
        return -1;
    }

    int r_len = 0;

    do{

        r_len = read(fd, r_buf, size);
        if (r_len > 0) {
            continue;
        } else if (r_len < 0) { //数据接收完成
            if(errno == EINTR) {
                printf("Epoll read EINTR 中断 : %d\n", errno);
                continue;
            }
            else if(errno == EAGAIN) {
                printf("Epoll read EAGAIN 没有数据可读 : %d\n", errno);
                return 1;
            }
            else if(errno == EWOULDBLOCK) {
                printf("Epoll read EWOULDBLOCK 网络阻塞 : %d\n", errno);
                return 1;
            } else {
                printf("Epoll read error %d\n", errno);
                struct epoll_event ev;
                ev.events = EPOLLERR;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
                close(fd);
                return -1;
            }

        } else if (r_len == 0) { //连接断开
            struct epoll_event ev;
            ev.events = EPOLLERR;
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
            printf("client have exit : %d\n", errno);
            close(fd);

            return -1;
        } else {
            readUntilEmpty(fd);
            return -1;
        }

    } while (1);
}

int dataSend(int fd, int epoll_fd, char* s_buf, int size) {
    if (size > 0) {
        int tlen = 0;
        int wlen = 0;
        while (tlen < size) {
            wlen = write(fd, s_buf + tlen, size -tlen);
            if (wlen > 0) {
                tlen += wlen;
            } else if (wlen < 0){
                if(errno == EINTR) {
                    printf("Epoll write EINTR 中断 : %d\n", errno);
                    continue;
                }
                else if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    printf("Epoll write EAGAIN 没有数据可写 : %d\n", errno);
                    continue;
                } else {
                    printf("Epoll write Error : %d\n", errno);
                    return -1;
                }
            } else if (wlen == 0) {
                struct epoll_event ev;
                ev.events = EPOLLERR;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
                printf("client have exit : %d\n", errno);
                close(fd);
                return -1;
            }
        }

        return 1;
    }

    return -1;
}

int main(int argc, char **args)
{
    //监听句柄
    int listen_fd;

    //连接句柄
    int conn_fd;

    //epoll句柄
    int epoll_fd;

    //服务器地址
    struct sockaddr_in s_addr;

    //客户端地址
    struct sockaddr_in c_addr;

    //服务器三元组设置
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(8008);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//    s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //创建监听sock
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket Error %d...\n" , errno );
        exit( EXIT_FAILURE );
    }

    //设置成非阻塞模式
    if(setnonblocking(listen_fd) == -1)
    {
        printf("Setnonblocking Error : %d\n", errno);
        exit( EXIT_FAILURE );
    }

    //允许端口重用
    int on = 1;
    if(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
    {
        printf("Setsockopt Error : %d\n", errno);
        exit( EXIT_FAILURE );
    }

    //绑定监听sock
    if(bind(listen_fd, (struct sockaddr*) &s_addr, sizeof(s_addr)) < 0)
    {
        printf("Bind Error : %d\n", errno);
        exit( EXIT_FAILURE );
    }

    //设置监听最大数
    if((listen(listen_fd, maxlisten)) < 0)
    {
        printf("Listen Error : %d\n", errno);
        exit( EXIT_FAILURE );
    }

    //epoll句柄初始化
    epoll_fd = epoll_create(MAXEPOLL);
    if(epoll_fd <= 0)
    {
        printf("Epoll Create Error : %d\n", errno);
        exit( EXIT_FAILURE );
    }

    //epoll 时间响应句柄
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.data.fd = listen_fd;
    event.events = EPOLLET | EPOLLIN;

    //注册epoll事件
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) < 0)
    {
        printf("Epoll Error : %d\n", errno);
        exit( EXIT_FAILURE );
    }

    int wait_fds;
    char r_buf[1024];
    char s_buf[1024];
    socklen_t len = sizeof(c_addr);
    struct epoll_event evs[MAXEPOLL];

    //循环等待事件响应
    while(1)
    {
        wait_fds = epoll_wait(epoll_fd, evs, MAXEPOLL, -1);
        if(wait_fds == -1)
        {
            printf( "Epoll Wait Error : %d\n", errno );
            exit( EXIT_FAILURE );
        }
        if(wait_fds == 0)
        {
            continue;
        }
        for(int i = 0; i < wait_fds; i++)
        {
            if(evs[i].data.fd == listen_fd)
            {
                bzero(&c_addr, sizeof(c_addr));
                while ((conn_fd = accept(listen_fd, (struct sockaddr*)&c_addr, &len)) > 0) {

                    //设置成非阻塞模式
                    if(setnonblocking(conn_fd) == -1)
                    {
                        printf("Setnonblocking Error : %d\n", errno);
                        exit( EXIT_FAILURE );
                    }

                    //加入epoll监听队列
                    event.data.fd = conn_fd;
                    event.events = EPOLLIN | EPOLLET;
                    if((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event)) < 0)
                    {
                        printf("Epoll add Error : %d\n", errno);
                        exit( EXIT_FAILURE);
                    }

                    printf("conn_fd = %d,ip = %s,port = %d\n",conn_fd, inet_ntoa(c_addr.sin_addr),c_addr.sin_port);
                }

                if(conn_fd < 0)
                {
                    if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR) {
                        printf("Accept Error : %d\n", errno);
                    }
                }
                continue;

            } else {
                if(evs[i].events & EPOLLIN) {
                    int fd = evs[i].data.fd;
                    if (fd == -1) {
                        printf("Epoll EPOLLIN Error : %d\n", errno);
                        exit(EXIT_FAILURE);
                    }
                    //读取数据
                    memset(r_buf, 0, sizeof(r_buf));
                    if (dataRead(fd, epoll_fd, r_buf, sizeof(r_buf)) > 0) {
                        event.events = EPOLLOUT;
                        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
                        printf("Client msg:%s\n", r_buf);
                        memset(s_buf, 0, sizeof(s_buf));
                        strcpy(s_buf, r_buf);
                        //memcpy(s_buf, r_buf, strlen(r_buf));
                    }

//                    int iRt = read(fd, r_buf, sizeof(r_buf));
//                    if(iRt < 0)
//                    {
//                        if(errno == EINTR) {
//                            printf("Epoll read EINTR 中断 : %d\n", errno);
//                        }
//                        else if(errno == EAGAIN) {
//                            printf("Epoll read EAGAIN 没有数据可读 : %d\n", errno);
//                        }
//                        else if(errno == EWOULDBLOCK) {
//                            printf("Epoll read EWOULDBLOCK 网络问题 : %d\n", errno);
//                        } else {
//                            printf("Epoll read error %d\n", errno);
//                            exit(EXIT_FAILURE);
//                        }
//                    }
//                    if(iRt == 0)
//                    {
//                        event.events = EPOLLERR;
//                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
//                        printf("client have exit : %d\n", errno);
//                        close(fd);
//                    } else {
//                        event.events = EPOLLOUT;
//                        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
//                        printf("Client msg:%s\n",r_buf);
//                        memset(s_buf, 0, sizeof(s_buf));
//                        memcpy(s_buf, r_buf, strlen(r_buf));
//                    }

                }

                if(evs[i].events & EPOLLOUT) {
                    int fd = evs[i].data.fd;
                    if(fd < 0)
                    {
                        printf("Epoll EPOLLOUT Error : %d\n", errno);
                        exit(EXIT_FAILURE);
                    }

                    //回传数据
                    if (dataSend(fd, epoll_fd, s_buf, strlen(s_buf)) > 0) {
                        event.events = EPOLLIN;
                        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
                        printf("Server msg:%s\n",s_buf);
                        memset(s_buf, 0, sizeof(s_buf));
                    }

//                    int ret = write(fd, s_buf, strlen(s_buf));
//                    if(ret < 0)
//                    {
//                        if(errno == EINTR) {
//                            printf("Epoll write EINTR 中断 : %d\n", errno);
//                        }
//                        else if(errno == EAGAIN) {
//                            printf("Epoll write EAGAIN 没有数据可读 : %d\n", errno);
//                        }
//                        else if(errno == EWOULDBLOCK) {
//                            printf("Epoll write EWOULDBLOCK 网络问题 : %d\n", errno);
//                        } else {
//                            printf("Epoll write Error : %d\n", errno);
//                            exit(EXIT_FAILURE);
//                        }
//                    }
//                    if(ret == 0)
//                    {
//                        event.events = EPOLLERR;
//                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
//                        printf("client have exit : %d\n", errno);
//                        close(fd);
//                    }
                }
            }

        }
    }


    close( listen_fd );
    return 0;
}




