/**********************************************************
 *  \file epoll_server.cpp
 *  \brief
 *  \note	注意事项：
 * 
 * \version 
 * * \author Allen.L
**********************************************************/
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

	//epoll 时间响应句柄
	struct  epoll_event ev;

	//服务器三元组设置
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(8000);
//	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//创建监听sock
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket Error...\n" , errno );
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

	ev.events = EPOLLET | EPOLLIN;
	ev.data.fd = listen_fd;

	//注册epoll事件
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0)
	{
		printf("Epoll Error : %d\n", errno);
		exit( EXIT_FAILURE );
	}

	int wait_fds;
	char buf[1024];
	socklen_t len = sizeof(c_addr);
	struct epoll_event evs[MAXEPOLL];
	struct epoll_event ev_remov;

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
				conn_fd = accept(listen_fd, (struct sockaddr*)&c_addr, &len);
				if(conn_fd < 0)
				{
					printf("Accept Error : %d\n", errno);
					exit( EXIT_FAILURE );
				}
				if(conn_fd == 0)
				{
					printf ( "%s\n" , strerror(errno) );
				}
				printf("conn_fd = %d,ip = %s,port = %d\n",conn_fd, inet_ntoa(c_addr.sin_addr),c_addr.sin_port);
				//设置成非阻塞模式
				if(setnonblocking(conn_fd) == -1)
				{
					printf("Setnonblocking Error : %d\n", errno);
					exit( EXIT_FAILURE );
				}

				ev.data.fd = conn_fd;
				ev.events = EPOLLIN | EPOLLET;
				if((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev)) < 0)
				{
					printf("Epoll add Error : %d\n", errno);
					exit( EXIT_FAILURE );
				}
			}

			else
			{
				if(evs[i].events & EPOLLIN)
				{
					conn_fd = evs[i].data.fd;
					if(conn_fd < 0)
					{
						printf("Epoll EPOLLIN Error : %d\n", errno);
						exit(EXIT_FAILURE);
					}
					//读取数据
					memset(buf, 0, sizeof(buf));
					int iRt = read(conn_fd, buf, sizeof(buf));
					if(iRt == -1)
					{
						printf("Epoll read Error : %d\n", errno);
						exit(EXIT_FAILURE);
					}
					if(iRt == 0)
					{
						evs[i].events = EPOLLERR;
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, &ev);
						printf("client have exit : %d\n");
						close(evs[i].data.fd);
					}
					ev.events = EPOLLOUT;
					epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn_fd, &ev);
					printf("Client msg:%s\n",buf);
				}

				else if(evs[i].events & EPOLLOUT)
				{
					conn_fd = evs[i].data.fd;
					if(conn_fd < 0)
					{
						printf("Epoll EPOLLOUT Error : %d\n", errno);
						exit(EXIT_FAILURE);
					}
					//回传数据
					int ret = write(conn_fd, buf, strlen(buf));
					if(ret == -1)
					{
						printf("Epoll write Error : %d\n", errno);
						exit(EXIT_FAILURE);
					}
					if(ret == 0)
					{
						evs[i].events = EPOLLERR;
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, &ev);
						printf("client have exit : %d\n");
						close(conn_fd);
					}
					ev.events = EPOLLIN;
					epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn_fd, &ev);
					memset(buf, 0, sizeof(buf));
				}
			}

		}
	}

	close( listen_fd );
	return 0;
}

