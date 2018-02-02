/**********************************************************
 *  \file socket_server.cpp
 *  \brief
 *  \note	注意事项： 
 * 
 * \version 
 * * \author Allen.L
**********************************************************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define DEFAULT_PORT 8000

int main(int argc, char **argv) {

	const int MAX_MSGSIZE = 1024;
	char r_buf[MAX_MSGSIZE];
	char s_buf[MAX_MSGSIZE];
	char addr_p[16];
	struct sockaddr_in ser_addr;
	struct sockaddr_in c_addr;
	static int sock_fd;
	static int connect_fd;

	if((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("socket make error!");
		exit(0);
	}

	int on(0);
	if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0) {
		printf("setsockopt error \n");
		exit(0);
	}

	bzero(&ser_addr,sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(DEFAULT_PORT);

	if(bind(sock_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr)) == -1) {
		printf("%d	%s	%d\n",sock_fd, ser_addr.sin_addr, ser_addr.sin_port);
		printf("socket bind error\n");
		exit(0);
	}

	if(listen(sock_fd, 10) == -1) {
		printf("socket listen error\n");
		exit(0);
	}

	socklen_t addr_size = sizeof(c_addr);
	connect_fd = accept(sock_fd, (struct sockaddr*)&c_addr, &addr_size);

	if(connect_fd < 0) {
		printf("socket accept error\n");
		exit(0);
	}

	inet_ntop(AF_INET,&c_addr.sin_addr,addr_p,16);
	printf("client IP is %s,port is %d\n",addr_p,ntohs(c_addr.sin_port));

	while(1) {
		memset(r_buf, 0, sizeof(r_buf));
		int len  = recv(connect_fd, r_buf, MAX_MSGSIZE-1,0);
		if(strcmp(r_buf,"exit")==0)
			break;
		fputs(r_buf, stdout);
		send(connect_fd, r_buf, len, 0);
	}

	close(connect_fd);
	close(sock_fd);
	return 0;
}
