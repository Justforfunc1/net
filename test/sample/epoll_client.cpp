/**********************************************************
 *  \file epoll_client.cpp
 *  \brief
 *  \note	注意事项：
 * 
 * \version 
 * * \author Allen.L
**********************************************************/

#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>


int main(int argc, char **args)
{
	int sock = socket( AF_INET, SOCK_STREAM,0 );
    if( sock< 0 )
	{
		printf( "Sock make Error : %d\n", errno );
		exit( EXIT_FAILURE );
	}

    struct sockaddr_in my_addr;
    //memset my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(8000);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int conn = connect(sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) ;
    if(conn != 0)
	{
		printf( "Sock conn Error : %d\n", errno );
		exit( EXIT_FAILURE );
	}

    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};
    fd_set rset;
    FD_ZERO(&rset);

    int nready = 0;
    int maxfd;
    int stdinof = fileno(stdin);
    if( stdinof > sock)
        maxfd = stdinof;
    else
        maxfd = sock;
    while(1)
    {
        FD_SET(sock, &rset);
        FD_SET(stdinof, &rset);
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if(nready == -1 )
            break;
        else if( nready == 0)
            continue;
        else
        {
            if( FD_ISSET(sock, &rset) )  //检测sock是否已经在集合rset里面。
            {
                int ret = read( sock, recvbuf, sizeof(recvbuf));  //读数据
                if( ret == -1)
				{
					printf( "Read Error : %d\n", errno );
					exit( EXIT_FAILURE );
				}
                else if( ret == 0)
                {
					printf("Server close\n");
                    close(sock);
                    break;
                }
                else
                {
                    fputs(recvbuf,stdout);    //输出数据
                    memset(recvbuf, 0, strlen(recvbuf));
                }
            }

            if( FD_ISSET(stdinof, &rset))  //检测stdin的文件描述符是否在集合里面
            {
                if(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
                {
                    int num = write(sock, sendbuf, strlen(sendbuf));  //写数据
                    memset(sendbuf, 0, sizeof(sendbuf));
                }
            }
        }
    }
    return 0;
}
