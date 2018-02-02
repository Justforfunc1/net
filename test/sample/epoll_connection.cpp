/**********************************************************
 * \file epoll_connection.cpp
 * \brief
 * \note	注意事项：
 *
 *\version 
 ** \author Allen.L
**********************************************************/
#include "epoll_connection.h"

using namespace lpsys;
using namespace std;
#define INIT_FILE "../conf/epoll.ini"

epoll_connection::epoll_connection():m_isRun(false),epoll_fd(0),listen_fd(0),conn_fd(0) {

	LPCONFIG_LOAD(INIT_FILE);
	string log_file = string(LPCONFIG_GETSTR("logging", "logger", "../logs/test_YYYYMMDD.log"));
	string log_level = string(LPCONFIG_GETSTR("logging", "level", "debug"));
	LPSYS_LOGGER_INIT(log_file, log_level, true);

	memset(&s_addr, 0, sizeof(s_addr));
	memset(&c_addr, 0, sizeof(c_addr));

}

epoll_connection::~epoll_connection() {

}

int epoll_connection::ep_setnonblocking(int fd)
{
	int ret = fcntl(fd,  F_SETFL, O_NONBLOCK );
		if(ret < 0)
		{
			printf("Setnonblocking error : %d\n", errno);
		}

	return fcntl(fd, F_SETFL, ret | O_NONBLOCK);
}


void epoll_connection::ep_config()
{

	LPLOG_INFO("------>EPOLL init start...");

	//ep_struct.ep_port = LPCONFIG_GETINT("listen", "port", -1);
	if (ep_struct.ep_port == -1) {
		LPLOG_WARNING("profile config lost[%s][%s]. use defalut value!", "listen", "port\n");

	}
	ep_struct.ep_port = 8000;

	//ep_struct.ep_listensize = LPCONFIG_GETINT("listen", "max_listen", -1);
	if (ep_struct.ep_listensize == -1) {
		LPLOG_WARNING("profile config lost[%s][%s]. use defalut value!", "listen", "max_listen\n");

	}
	ep_struct.ep_listensize = 10;

	//ep_struct.ep_eventsize = LPCONFIG_GETINT("listen", "max_events", -1);
	if (ep_struct.ep_eventsize == -1) {
		LPLOG_WARNING("profile config lost[%s][%s]. use defalut value!", "listen", "max_events\n");

	}
	ep_struct.ep_eventsize = 1000;

	LPLOG_INFO("------>EPOLL init is OK!");
	m_isRun = true;

}

int epoll_connection::ep_event_ctl(int op, int fd, event_type_t filter)
{
	memset(&ev, 0, sizeof(struct epoll_event));
	if (filter == FDEVENT_READ)
		ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;  //EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET
	else if (filter == FDEVENT_WRITE)
		ev.events = EPOLLOUT | EPOLLERR | EPOLLHUP; //EPOLLOUT | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET
	else
		ev.events = 0;

	ev.data.fd = fd;

	return epoll_ctl(epoll_fd, op, fd, &ev);
}

int epoll_connection::ep_event_insert(int fd, event_type_t filter)
{
	return ep_event_ctl(EPOLL_CTL_ADD, fd, filter);
}

int epoll_connection::ep_event_update(int fd, event_type_t filter)
{
	return ep_event_ctl(EPOLL_CTL_MOD, fd, filter);
}

int epoll_connection::ep_event_delete(int fd)
{
	return ep_event_ctl(EPOLL_CTL_DEL, fd, FDEVENT_DELETE);
}

void epoll_connection::ep_event_destroy()
{
	close(epoll_fd);
}

int epoll_connection::ep_startupServer()
{
	LPLOG_INFO("------>EPOLL startup.....");
	ep_config();

	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(uint16_t(ep_struct.ep_port));
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket Error  : %d\n" , errno );
		exit( EXIT_FAILURE );
	}

	if(ep_setnonblocking(listen_fd) == -1)
	{
		printf("Setnonblocking Error : %d\n", errno);
		exit( EXIT_FAILURE );
	}

	int on = 1;
	if(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
	{
		printf("Setsockopt Error : %d\n", errno);
		exit( EXIT_FAILURE );
	}

	if(bind(listen_fd, (struct sockaddr*) &s_addr, sizeof(s_addr)) < 0)
	{
		printf("Bind Error : %d\n", errno);
		exit( EXIT_FAILURE );
	}

	if((listen(listen_fd, ep_struct.ep_listensize)) < 0)
	{
		printf("Listen Error : %d\n", errno);
		exit( EXIT_FAILURE );
	}

	epoll_fd = epoll_create(MAX_EVENTS);
	if(epoll_fd <= 0)
	{
		printf("Epoll Create Error : %d\n", errno);
		exit( EXIT_FAILURE );
	}

	if(ep_event_insert(listen_fd, FDEVENT_READ) < 0)
	{
		printf("Epoll insert Error : %d\n", errno);
		exit( EXIT_FAILURE );
	}

	int wait_fds;
	char buf[BUFSIZE];

	while(m_isRun)
	{
		wait_fds = epoll_wait(epoll_fd, evs, ep_struct.ep_eventsize-1, -1);
		if(wait_fds == -1)
		{
			printf( "Epoll Wait Error : %d\n", errno );
			exit( EXIT_FAILURE );
		}

/*		if(wait_fds == 0)
		{
			continue;
		}
*/
		for(int i = 0; i < wait_fds; i++)
		{
			if(evs[i].data.fd == listen_fd)
			{
				ep_accept();
				continue;
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
					ep_dataRecv(buf, conn_fd);

				}
				if(evs[i].events & EPOLLOUT)
				{
					conn_fd = evs[i].data.fd;
					if(conn_fd < 0)
					{
						printf("Epoll EPOLLOUT Error : %d\n", errno);
						exit(EXIT_FAILURE);
					}
					ep_dataSend(buf, conn_fd);
				}

			}

		}

	}

	ep_event_destroy();
	LPLOG_INFO("------>EPOLL exit.....");
	return 0;
}

int epoll_connection::ep_accept()
{

	socklen_t len = sizeof(c_addr);
	bzero(&c_addr, sizeof(c_addr));


	while((conn_fd = accept(listen_fd, (struct sockaddr*)&c_addr, &len)) > 0)
	{
		if(ep_setnonblocking(conn_fd) == -1)
		{
			printf("Setnonblocking Error : %d\n", errno);
			exit( EXIT_FAILURE );
		}
		if(ep_event_insert(conn_fd, FDEVENT_READ) < 0)
		{
			printf("Epoll insert Error : %d\n", errno);
			exit( EXIT_FAILURE );
		}

		printf("conn_fd = %d,ip = %s,port = %d\n",conn_fd, inet_ntoa(c_addr.sin_addr),c_addr.sin_port);
		LPLOG_DEBUG("conn_fd = %d,ip = %s,port = %d\n",conn_fd, inet_ntoa(c_addr.sin_addr),c_addr.sin_port);

	}
	if (conn_fd == -1)
	{
		if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
		{
			printf("Epoll accept Error : %d\n", errno);
			perror("accept");
			exit( EXIT_FAILURE );
		}

		return -1;

	}


/*	conn_fd = accept(listen_fd, (struct sockaddr*)&c_addr, &len);
	if (conn_fd == -1)
	{
		if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
			printf("Epoll accept Error : %d\n", errno);
			exit( EXIT_FAILURE );
	}

	if(ep_setnonblocking(conn_fd) == -1)
	{
		printf("Setnonblocking Error : %d\n", errno);
		exit( EXIT_FAILURE );
	}

	if(ep_event_insert(conn_fd, FDEVENT_READ) < 0)
	{
		printf("Epoll insert Error : %d\n", errno);
		exit( EXIT_FAILURE );
	}
*/

	return conn_fd;
}


int epoll_connection::ep_dataRecv(char* rbuf, int fd)
{
	LPLOG_INFO("------>dataRecv start...");

	int n = 0;
	int iRt = 0;
	memset(rbuf, 0, BUFSIZE);

	while((iRt = read(fd, rbuf+n, BUFSIZE-1)) > 0)
	{
		n += iRt;
	}

	if(iRt == -1 && errno != EAGAIN)
	{
		printf("Epoll read Error : %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if(iRt == 0)
	{
		ep_event_delete(fd);
		printf("client[%s] have exit :%d\n", inet_ntoa(c_addr.sin_addr), errno);
		close(fd);
	}

	printf("Client[%s]  msg:%s", inet_ntoa(c_addr.sin_addr), rbuf);
	ep_event_update(fd,FDEVENT_WRITE);
	LPLOG_INFO("------>dataRecv ok...");

	return iRt;

}

int epoll_connection::ep_dataSend(char* sbuf, int fd)
{
	LPLOG_INFO("------>dataSend start...");
	int len = strlen(sbuf);
	int n = len;
	int ret = 0;
	while(n > 0)
	{
		ret = write(conn_fd, sbuf+len-n, n);
		if (ret < n)
		{
			if (ret == -1 && errno != EAGAIN)
			{
				printf("Epoll write Error : %d\n", errno);
				exit(EXIT_FAILURE);
			}
			break;
		}
			n -= ret;
	}
	if(ret == 0)
	{
		ep_event_delete(fd);
		printf("client[%s] have exit : %d\n",inet_ntoa(c_addr.sin_addr), errno);
		close(fd);
	}
	ep_event_update(fd,FDEVENT_READ);
	LPLOG_INFO("------>dataSend ok...");
	return ret;

}

