#pragma once
#include <sys/epoll.h>
#include <vector>
#include "Macros.h"
class Channel;
class Epoll
{
private:
    int epfd_;
    struct epoll_event *events_;
public:
    Epoll();
    ~Epoll();

    DISALLOW_COPY_AND_MOVE(Epoll);
    void addFd(int fd, uint32_t op);
    void updateChannel(Channel*);
    void deleteChannel(Channel*);
    // std::vector<epoll_event> poll(int timeout = -1);
    std::vector<Channel*> poll(int timeout = -1);
};