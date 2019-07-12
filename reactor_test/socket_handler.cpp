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

int SocketHandler::handle_read() {
    memset(_buf, 0, MAX_SIZE);
    if (_buf == nullptr) {
        //清空socket缓冲区
        readUntilEmpty(_socket_fd);
        printf("buf is nullptr");
        return -1;
    }

    int r_len = 0;
    do{
        r_len = read(_socket_fd, _buf, MAX_SIZE);
        if (r_len > 0) {
            continue;
        } else if (r_len < 0) { //数据接收完成
            if(errno == EINTR) {
                printf("Epoll read EINTR 中断 : %d\n", errno);
                continue;
            }
            else if(errno == EAGAIN) {
                printf("Epoll read EAGAIN 没有数据可读 : %d\n", errno);
                return 1;
            }
            else if(errno == EWOULDBLOCK) {
                printf("Epoll read EWOULDBLOCK 网络阻塞 : %d\n", errno);
                return 1;
            } else {
                printf("Epoll read error %d\n", errno);
                handle_error();
                return -1;
            }

        } else if (r_len == 0) { //连接断开
            printf("client [%d] have exit : %d\n", _socket_fd, errno);
            close(_socket_fd);
            return -1;
        } else {
            readUntilEmpty(_socket_fd);
            return -1;
        }

    } while (1);

}

int SocketHandler::handle_write() {
    int size = strlen(_buf);
    if (size > 0) {
        int tlen = 0;
        int wlen = 0;
        printf("client msg:%s\n", _buf);
        printf("socket [%d] send msg size : %d\n", _socket_fd, (int)strlen(_buf));
        while (tlen < size) {
            wlen = write(_socket_fd, _buf + tlen, size -tlen);
            if (wlen > 0) {
                tlen += wlen;
            } else if (wlen < 0){
                if(errno == EINTR) {
                    printf("Epoll write EINTR 中断 : %d\n", errno);
                    continue;
                }
                else if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    printf("Epoll write EAGAIN 没有数据可写 : %d\n", errno);
                    continue;
                } else {
                    printf("Epoll write Error : %d\n", errno);
                    return -1;
                }
            } else if (wlen == 0) {
                printf("client [%d] have exit : %d\n", _socket_fd, errno);
                close(_socket_fd);
                return -1;
            }
        }
        printf("server send msg [%s] success \n",_buf);
        return 1;
    } else {
        printf("Epoll write s_buf is null. fd [%d],  errno : %d\n", _socket_fd, errno);
    }

    return -1;
}

void SocketHandler::handle_error() {
    Reactor& r = Reactor::get_instance();
    r.remove(this);
}


