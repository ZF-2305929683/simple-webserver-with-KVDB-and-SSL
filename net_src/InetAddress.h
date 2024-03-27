#pragma once
#include <arpa/inet.h>
#include "Macros.h"
class InetAddress
{
public:
    
    socklen_t addr_len;
    InetAddress();
    InetAddress(const char* ip, uint16_t port);
    ~InetAddress();

    DISALLOW_COPY_AND_MOVE(InetAddress);

    void SetAddr(sockaddr_in addr);
    sockaddr_in GetAddr();
    const char *GetIp();
    uint16_t GetPort();
private:
    struct sockaddr_in addr_;
};