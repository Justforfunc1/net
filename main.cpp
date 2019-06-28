#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <errno.h>
#include "reactor.h"
#include "event_handler.h"
#include "accept_handler.h"
#include "event.h"
#include <fcntl.h>


//设置非阻塞模式
int setnonblocking( int fd )
{
    int ret = fcntl(fd,  F_SETFL, O_NONBLOCK );
    if(ret < 0)
        printf("Setnonblocking error : %d\n", errno);
    return 0;
}

int main() {
    int listenfd = -1;

    struct sockaddr_in seraddr;
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(8008);
//    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //创建监听sock
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "socket error " << errno << std::endl;
        exit(-1);
    }

    //设置成非阻塞模式
    if(setnonblocking(listenfd) == -1)
    {
        printf("Setnonblocking Error : %d\n", errno);
        exit( EXIT_FAILURE );
    }

    //允许端口重用
    int on = 1;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
    {
        printf("Setsockopt Error : %d\n", errno);
        exit( EXIT_FAILURE );
    }

    //绑定监听sock
    if (bind(listenfd, (struct sockaddr*)&seraddr, sizeof(seraddr)) < 0) {
        std::cerr << "bind error " << errno << std::endl;
        exit(-2);
    }

    //设置监听最大数
    if (listen(listenfd, 5) < 0) {
        std::cerr << "listen error " << errno << std::endl;
        exit(-3);
    }


    Reactor& actor = Reactor::get_instance();
    EventHandler* handler = new AcceptHandler(listenfd);
    actor.regist(handler, AcceptEvent);
    actor.dispatch(100);


    return 0;
}

