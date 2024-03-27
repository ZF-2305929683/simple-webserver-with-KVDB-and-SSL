#pragma once
#include "Macros.h"
#include <sys/epoll.h>
#include <functional>

class Epoll;
class EventLoop;
class Channel
{
private:
    EventLoop *loop_;
    int fd_;
    uint32_t events_;
    uint32_t ready_;
    bool inEpoll_;
    std::function<void()> readcallback_;
    std::function<void()> writecallback_;
public:
    Channel(EventLoop *loop, int fd);
    ~Channel();

    DISALLOW_COPY_AND_MOVE(Channel);

    void handleEvent();
    void enableReading();

    int getFd();
    uint32_t getEvents();
    uint32_t getReady();
    bool getInEpoll();
    void setInEpoll(bool _in = true);
    void useET();

    // void setEvents(uint32_t);
    void setReady(uint32_t);
    void setReadCallback(std::function<void()> const &cb);
    void setWriteCallback(std::function<void()> const &cb);
};