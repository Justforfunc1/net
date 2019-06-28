//
// Created by Administrator on 2018/11/28.
//

#include "socket_handler.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <new>
#include "reactor.h"
#include <iostream>

SocketHandler::SocketHandler(Handle fd) :
        _socket_fd(fd) {
    _buf = new (std::nothrow)char[MAX_SIZE];
    assert(_buf != NULL);
    memset(_buf, 0, MAX_SIZE);
}

SocketHandler::~SocketHandler() {
    close(_socket_fd);
    delete[] _buf;
}

int SocketHandler::handle_accept() {
    // do nothing
    return 0;
}

void SocketHandler::handle_read() {
    memset(_buf, 0, MAX_SIZE);
    if (read(_socket_fd, _buf, MAX_SIZE) > 0) {
        std::cout << "write : [" <<_buf << "]" << std::endl;
    }
    handle_error();
}

void SocketHandler::handle_write() {

    if(write(_socket_fd, _buf, strlen(_buf)) > 0) {
        std::cout <<  "send success!" << std::endl;
    }
    handle_error();
}

void SocketHandler::handle_error() {
    Reactor& r = Reactor::get_instance();
    r.remove(this);
}


