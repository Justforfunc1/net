/**********************************************************
 *  \file socket_client.cpp
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
	struct sockaddr_in s_addr;
	static int sock_fd;

	if((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("socket make error!\n");
		exit(0);
	}

	memset(&s_addr,0,sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(DEFAULT_PORT);
	s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if((sock_fd = connect(sock_fd, (struct sockaddr*)&s_addr, sizeof(s_addr))) == -1) {
		printf("connect socket error\n");
		exit(0);
	}

	while(fgets(s_buf, sizeof(s_buf), stdin) != NULL) {

		send(sock_fd, s_buf, strlen(s_buf), 0);

		if((strcmp(s_buf,"exit"))==0) {
				break;
		}
		recv(sock_fd, r_buf, sizeof(r_buf), 0);
		fputs(r_buf,stdout);

		memset(&s_buf, 0, sizeof(s_buf));
		memset(&r_buf, 0, sizeof(r_buf));

	}

	close(sock_fd);
	return 0;
}
