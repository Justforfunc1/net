//
// Created by Administrator on 2018/11/28.
//

#include "accept_handler.h"
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <new>
#include <assert.h>
#include "event_handler.h"
#include "reactor.h"
#include "socket_handler.h"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>

AcceptHandler::AcceptHandler(Handle fd) : _listen_fd(fd) {
    // do nothing
}

AcceptHandler::~AcceptHandler() {
    close(_listen_fd);
}

int AcceptHandler::handle_accept() {
    struct sockaddr_in c_addr;
    socklen_t len = sizeof(c_addr);
    Handle conn_fd;
//    Handle fd  = accept(_listen_fd, (struct sockaddr*)&c_addr, &addr_size);
//    setnonblocking(fd);
//    inet_ntop(AF_INET,&c_addr.sin_addr,addr_p,16);
//    std::cout << "accept conn success! fd:[" << fd << "]" << std::endl;
//    printf("client ip : %s,port : %d\n",addr_p,ntohs(c_addr.sin_port));

    while ((conn_fd = accept(_listen_fd, (struct sockaddr*)&c_addr, &len)) > 0) {

        //设置成非阻塞模式
        if(setnonblocking(conn_fd) == -1)
        {
            printf("Setnonblocking Error : %d\n", errno);
            exit( EXIT_FAILURE );
        }

        EventHandler *h = new SocketHandler(conn_fd);
        Reactor& r = Reactor::get_instance();
        r.regist(h, ReadEvent);

//        //加入epoll监听队列
//        event.data.fd = conn_fd;
//        event.events = EPOLLIN | EPOLLET;
//        if((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event)) < 0)
//        {
//            printf("Epoll add Error : %d\n", errno);
//            exit( EXIT_FAILURE);
//        }

        printf("conn_fd = %d,ip = %s,port = %d\n",conn_fd, inet_ntoa(c_addr.sin_addr),c_addr.sin_port);
    }

    return conn_fd;
}

int AcceptHandler::handle_read() {
    // do nothing
    return 0;
}

int AcceptHandler::handle_write() {
    // do nothing
    return 0;
}

void AcceptHandler::handle_error() {
    // do nothing
}


