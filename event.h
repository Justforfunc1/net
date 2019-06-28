//
// Created by Administrator on 2018/11/28.
//



#ifndef EVENT_H
#define EVENT_H

typedef unsigned int Event;

enum {
    ReadEvent  = 0x01,
    WriteEvent = 0x02,
    ErrorEvent = 0x04,
    AcceptEvent = 0x08,
    EventMask  = 0xff
};

#endif
