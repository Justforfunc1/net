//
// Created by Administrator on 2018/11/28.
//

#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "event.h"

typedef int Handle;

class EventHandler {
public:
    EventHandler() {}
    virtual ~EventHandler() {}
    virtual Handle get_handle() const = 0;
    virtual int handle_accept() = 0;
    virtual int handle_read() = 0;
    virtual int handle_write() = 0;
    virtual void handle_error() = 0;
};

#endif
