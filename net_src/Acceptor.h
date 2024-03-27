#pragma once
#include "Macros.h"
#include <functional>
class Socket;
class InetAddress;
class Channel;
class EventLoop;

class Accepter
{
private:
    EventLoop *loop_;
    Socket *sock_;
    InetAddress *addr_;
    Channel *acceptChannel_;
    std::function<void(Socket*)> newConnectionCallback_;
public:
    explicit Accepter(EventLoop*);
    ~Accepter();

    DISALLOW_COPY_AND_MOVE(Accepter);

    void acceptConnection();
    void setNewConnectionCallback(std::function<void(Socket*)>);
};


