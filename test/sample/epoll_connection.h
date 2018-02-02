/**********************************************************
 * \file epoll_connection.h
 * \brief
 * \note	注意事项：
 *
 *\version 
 ** \author Allen.L
**********************************************************/
#ifndef _EPOLL_CONNECTION_H
#define _EPOLL_CONNECTION_H

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

#include "lpsys/lpsys.h"

#define MAX_EVENTS 1000
#define MAX_LISTEN 10
#define BUFSIZE 1024

typedef enum {
		FDEVENT_READ, FDEVENT_WRITE, FDEVENT_DELETE
} event_type_t;

typedef struct epoll_struct
{
	int ep_port;
	int ep_listensize;
	int ep_eventsize;

}epoll_struct_t;


class epoll_connection
{

private:
	bool m_isRun;
	int epoll_fd;
	int listen_fd;
	int conn_fd;
	struct sockaddr_in s_addr;
	struct sockaddr_in c_addr;
	struct epoll_event ev;
	struct epoll_event evs[MAX_EVENTS];
	epoll_struct_t  ep_struct;


	/// \brief ep_accept 接收新连接
	int ep_accept();

	/// \brief ep_event_ctl  epoll时间处理
	int ep_event_ctl(int op, int fd, event_type_t filter);

	/// \brief ep_event_insert 加入事件监控
	int ep_event_insert(int fd, event_type_t filter);

	/// \brief ep_event_update 更新事件监控
	int ep_event_update(int fd, event_type_t filter);

	/// \brief ep_event_delete 删除事件监控
	int ep_event_delete(int fd);

	/// \brief ep_event_destroy 关闭事件fd
	void ep_event_destroy();


public:
	epoll_connection();
	~epoll_connection();

	/// \brief startupEpoll 启动epoll
	int ep_startupServer();

	/// \brief setnonblocking 设置非阻塞模式
	int ep_setnonblocking(int fd);

	/// \brief epoll_config 配置文件
	void ep_config();

	/// \brief dataSend 发送数据
	int ep_dataSend(char* sbuf, int fd);

	/// \brief dataRecv 接收数据
	int ep_dataRecv(char* rbuf, int fd);

};

#endif


