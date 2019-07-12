//
// Created by Administrator on 2018/11/28.
//

#include "reactor.h"
#include <assert.h>
#include <new>
#include "epoll_demultiplexer.h"

Reactor Reactor::reactor;

Reactor& Reactor::get_instance() {
    return reactor;
}

Reactor::Reactor() {
    _demultiplexer = new (std::nothrow)EpollDemultiplexer();
    assert(_demultiplexer != nullptr);
}

Reactor::~Reactor() {
    std::map<Handle, EventHandler*>::iterator iter = _handlers.begin();
    for(; iter != _handlers.end(); ++iter) {
        delete iter->second;
    }

    if (_demultiplexer != nullptr) {
        delete _demultiplexer;
    }
}


int Reactor::regist(EventHandler* handler, Event evt) {
    int handle = handler->get_handle();
    if (_handlers.find(handle) == _handlers.end()) {
        _handlers.insert(std::make_pair(handle, handler));
    }
    return _demultiplexer->regist(handle, evt);
}

void Reactor::remove(EventHandler* handler) {
    int handle = handler->get_handle();
    // not check?
    _demultiplexer->remove(handle);

    std::map<Handle, EventHandler*>::iterator iter = _handlers.find(handle);
    delete iter->second;
    _handlers.erase(iter);
}

void Reactor::dispatch(int timeout) {
    _demultiplexer->wait_event(_handlers, timeout);
}


