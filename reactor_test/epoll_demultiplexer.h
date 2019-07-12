//
// Created by Administrator on 2018/11/28.
//

#include <map>
#include "event_handler.h"
#include "event.h"
#include "event_demultiplexer.h"
#include <sys/epoll.h>

class EpollDemultiplexer : public EventDemultiplexer {
public:
    EpollDemultiplexer();
    virtual ~EpollDemultiplexer();
    virtual int wait_event(std::map<Handle, EventHandler*>& handlers, int timeout = 0);
    virtual int regist(Handle handle, Event evt);
    virtual int remove(Handle handle);
private:
    int _max_fd;
    int _epoll_fd;
    int _listen_fd;
};

