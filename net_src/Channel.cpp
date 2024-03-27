#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"
#include <unistd.h>
#include <sys/epoll.h>

Channel::Channel(EventLoop *loop, int fd) 
: loop_(loop), fd_(fd), events_(0), ready_(0), inEpoll_(false){
}

Channel::~Channel()
{
    if(fd_ != -1)
    {
        close(fd_);
        fd_ = -1;
    }
}

void Channel::handleEvent()
{
    if(ready_ & (EPOLLIN | EPOLLPRI))
    {
        readcallback_();
    }
    if(ready_ & (EPOLLOUT))
    {
        writecallback_();
    }
}

void Channel::enableReading()
{
    events_ = EPOLLIN | EPOLLET;
    loop_->updateChannel(this);
}

int Channel::getFd()
{
    return fd_;
}

uint32_t Channel::getEvents()
{
    return events_;
}
uint32_t Channel::getReady()
{
    return ready_;
}

bool Channel::getInEpoll()
{
    return inEpoll_;
}

void Channel::setInEpoll(bool in)
{
    inEpoll_ = in;
}


void Channel::setReady(uint32_t ev)
{
    ready_ = ev;
}

void Channel::setReadCallback(std::function<void()> const &cb)
{
    readcallback_ = cb;
}

void Channel::setWriteCallback(std::function<void()> const &cb) 
{ 
    writecallback_ = cb; 
}

void Channel::useET()
{
    events_ |= EPOLLET;
    loop_->updateChannel(this);
}