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
#define MAXEPOLL	1024

EpollDemultiplexer::EpollDemultiplexer() : _max_fd(0) {
    _epoll_fd = epoll_create(MAXEPOLL);
}

EpollDemultiplexer::~EpollDemultiplexer() {
    close(_epoll_fd);
}

int EpollDemultiplexer::wait_event(std::map<Handle, EventHandler*>& handlers, int timeout) {

    //std::vector<struct epoll_event> events(_max_fd);
    struct epoll_event events[MAXEPOLL];
    int wait_fds = 0;
    bool isRun = true;
    Handle handle;

    while (isRun) {
        wait_fds = epoll_wait(_epoll_fd, events, MAXEPOLL, -1);
        if (wait_fds < 0) {
            std::cout << "epoll_wait error!" << errno << std::endl;
            return wait_fds;
        }
        if(wait_fds == 0)
        {
            continue;
        }
        for (int i = 0; i < wait_fds; i++) {
            handle = events[i].data.fd;
            if (handle == _listen_fd) {
                if(handlers[handle]->handle_accept() < 0)
                {
                    if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR) {
                        printf("Accept Error : %d\n", errno);
                    }
                }
                continue;
            } else {
                if ((EPOLLIN) & events[i].events) {
                    assert(handlers[handle] != NULL);
                    if ((handlers[handle])->handle_read() > 0) {
//                        (handlers[handle])->handle_write();
                        struct epoll_event ev;
                        ev.data.fd = handle;
                        ev.events = EPOLLOUT;
                        if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, handle, &ev) != 0) {
                            if (errno != ENOENT) {
                                std::cout << "epoll_ctl mod write error " << errno << std::endl;
                            }
                        }
                    }
                }
                if (EPOLLOUT & events[i].events) {
                    if((handlers[handle])->handle_write() > 0) {
                        struct epoll_event ev;
                        ev.data.fd = handle;
                        ev.events = EPOLLIN | EPOLLET;
                        if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, handle, &ev) != 0) {
                            if (errno != ENOENT) {
                                std::cout << "epoll_ctl mod read error " << errno << std::endl;
                            }
                        }
                    }
                }
                if ((EPOLLHUP|EPOLLERR) & events[i].events) {
                    assert(handlers[handle] != NULL);
                    (handlers[handle])->handle_error();
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
        _listen_fd = handle;
        ev.events = EPOLLET | EPOLLIN;
    }

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, handle, &ev) != 0) {
        if (errno == ENOENT) {
            if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _listen_fd, &ev) != 0) {
                std::cout << "epoll_ctl add second error " << errno << std::endl;
                return -errno;
            }
            ++_max_fd;
        } else {
            std::cout << "epoll_ctl add first error " << errno << std::endl;
            ++_max_fd;
        }
    }

    return 0;
}

int EpollDemultiplexer::remove(Handle handle) {
    struct epoll_event ev;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, handle, &ev) != 0) {
        std::cout << "epoll_ctl del error" << errno << std::endl;
        return -errno;
    }
    --_max_fd;
    return 0;
}
