//
// Created by Administrator on 2018/11/28.
//

#include "event_handler.h"
#include "event.h"
#include <sys/socket.h>

class SocketHandler : public EventHandler {
public:
    SocketHandler(Handle fd);
    virtual ~SocketHandler();
    virtual Handle get_handle() const {
        return _socket_fd;
    }
    virtual int handle_accept();
    virtual int handle_read();
    virtual int handle_write();
    virtual void handle_error();

    inline void readUntilEmpty(int conn) {
        int r_len;
        char buf[1024];
        do {
            r_len = recv(conn, buf, 1024, 0);
        } while (r_len > 0);
    }

private:
    Handle _socket_fd;
    char* _buf;
    static const int MAX_SIZE = 1024;
};

