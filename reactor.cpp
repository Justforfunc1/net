//
// Created by Administrator on 2018/11/28.
//

#include "reactor.h"
#include <assert.h>
#include <new>

Reactor Reactor::reactor;

Reactor& Reactor::get_instance() {
    return reactor;
}

Reactor::Reactor() {
    _reactor_impl = new (std::nothrow)ReactorImpl();
    assert(_reactor_impl != NULL);
}

Reactor::~Reactor() {
    if (_reactor_impl != NULL) {
        delete _reactor_impl;
        _reactor_impl = NULL;
    }
}


int Reactor::regist(EventHandler* handler, Event evt) {
    return _reactor_impl->regist(handler, evt);
}

void Reactor::remove(EventHandler* handler) {
    return _reactor_impl->remove(handler);
}

void Reactor::dispatch(int timeout) {
    return _reactor_impl->dispatch(timeout);
}


