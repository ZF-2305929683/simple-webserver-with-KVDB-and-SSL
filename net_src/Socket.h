#pragma once
#include "Macros.h"
#include <arpa/inet.h>

class InetAddress;
class Socket
{
private:
    int fd_;
public:
    Socket();
    explicit Socket(int fd);
    ~Socket();

    DISALLOW_COPY_AND_MOVE(Socket);

    void bind(InetAddress* addr);
    void listen();

    void Connect(InetAddress *addr);
    void Connect(const char *ip, uint16_t port);
    int accept(InetAddress* addr);

    int getFd();

    void setnonblocking();
    bool Isnonblocking();
};