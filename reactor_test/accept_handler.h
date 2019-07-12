//
// Created by Administrator on 2018/11/28.
//

#include "event_handler.h"
#include "event.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

class AcceptHandler : public EventHandler {
public:
    AcceptHandler(Handle fd);
    virtual ~AcceptHandler();
    virtual Handle get_handle() const {
        return _listen_fd;
    }
    virtual int handle_accept();
    virtual int handle_read();
    virtual int handle_write();
    virtual void handle_error();

    inline int setnonblocking( int fd )
    {
        int ret = fcntl(fd,  F_SETFL, O_NONBLOCK );
        if(ret < 0)
            printf("Setnonblocking error : %d\n", errno);
        return 0;
    }

private:
    Handle _listen_fd;
};

