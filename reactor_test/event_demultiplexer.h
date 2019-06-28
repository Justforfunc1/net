//
// Created by Administrator on 2018/11/28.
//

#ifndef EVENT_DEMULTIPLEXER_H
#define EVENT_DEMULTIPLEXER_H

#include <map>
#include "event_handler.h"
#include "event.h"

class EventDemultiplexer {
public:
    EventDemultiplexer() {}
    virtual ~EventDemultiplexer()  {}
    virtual int wait_event(std::map<Handle, EventHandler*>& handlers, int timeout = 0) = 0;
    virtual int regist(Handle handle, Event evt) = 0;
    virtual int remove(Handle handle) = 0;
};

#endif

