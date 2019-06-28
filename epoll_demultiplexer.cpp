//
// Created by Administrator on 2018/11/28.
//

#include <vector>
#include <sys/epoll.h>
#include <iostream>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include "reactor.h"

#include "epoll_demultiplexer.h"
#include "socket_handler.h"

EpollDemultiplexer::EpollDemultiplexer() : _max_fd(0) {
    _epoll_fd = epoll_create(1024);
}

EpollDemultiplexer::~EpollDemultiplexer() {
    close(_epoll_fd);
}

int EpollDemultiplexer::wait_event(std::map<Handle, EventHandler*>& handlers, int timeout) {

    //std::vector<struct epoll_event> events(_max_fd);
    int num = 0;
    bool isRun = true;
    Handle conn_fd;
    Handle handle;

    while (isRun) {
        num = epoll_wait(_epoll_fd, events, _max_fd, -1);
        if (num < 0) {
            std::cout << "epoll_wait error!" << std::endl;
            return num;
        }
        for (int i = 0; i < num; ++i) {
            handle = events[i].data.fd;
            if (handle == _listen_fd) {
                conn_fd = handlers[handle]->handle_accept();
                EventHandler *h = new SocketHandler(conn_fd);
                Reactor& r = Reactor::get_instance();
                r.regist(h, ReadEvent);
            }
            if ((EPOLLHUP|EPOLLERR) & events[i].events) {
                assert(handlers[handle] != NULL);
                (handlers[handle])->handle_error();
            } else {
                if ((EPOLLIN) & events[i].events) {
                    assert(handlers[handle] != NULL);
                    (handlers[handle])->handle_read();
                    regist(handle, WriteEvent);
                }
                if (EPOLLOUT & events[i].events) {
                    (handlers[handle])->handle_write();
                    regist(handle, ReadEvent);
                }
            }
        }
    }

    return 0;
}

int EpollDemultiplexer::regist(Handle handle, Event evt) {

    struct epoll_event ev;
    ev.data.fd = handle;
    if (evt & ReadEvent) {
        ev.events = EPOLLIN | EPOLLET;
    }
    if (evt & WriteEvent) {
        ev.events = EPOLLOUT;
    }
    if (evt & AcceptEvent) {
        _listen_fd = ev.data.fd;
        ev.events = EPOLLET | EPOLLIN;
    }

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, handle, &ev) != 0) {
        if (errno == ENOENT) {
            if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, handle, &ev) != 0) {
                std::cerr << "epoll_ctl add error " << errno << std::endl;
                return -errno;
            }
            ++_max_fd;
        } else {
            ++_max_fd;
        }
    }
    return 0;
}

int EpollDemultiplexer::remove(Handle handle) {
    struct epoll_event ev;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, handle, &ev) != 0) {
        std::cerr << "epoll_ctl del error" << errno << std::endl;
        return -errno;
    }
    --_max_fd;
    return 0;
}
