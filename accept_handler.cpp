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
    char addr_p[16];
    struct sockaddr_in c_addr;
    socklen_t addr_size = sizeof(c_addr);
    Handle fd  = accept(_listen_fd, (struct sockaddr*)&c_addr, &addr_size);
    setnonblocking(fd);
    inet_ntop(AF_INET,&c_addr.sin_addr,addr_p,16);
    std::cout << "accept conn success! fd:[" << fd << "]" << std::endl;
    printf("client ip : %s,port : %d\n",addr_p,ntohs(c_addr.sin_port));
    return fd;
}

void AcceptHandler::handle_read() {
    // do nothing
}

void AcceptHandler::handle_write() {
    // do nothing
}

void AcceptHandler::handle_error() {
    // do nothing
}


