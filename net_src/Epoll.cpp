#include "Epoll.h"
#include "util.h"
#include "Channel.h"
#include <unistd.h>
#include <string.h>

#define MAX_EVENTS 1000

Epoll::Epoll() : epfd_(-1), events_(nullptr)
{
    epfd_ = epoll_create1(0);
    errif(epfd_ == -1, "epoll create error");
    events_ = new epoll_event[MAX_EVENTS];
    bzero(events_, sizeof(*events_) * MAX_EVENTS);
}

Epoll::~Epoll()
{
    if(epfd_ != -1){
        close(epfd_);
        epfd_ = -1;
    }
    delete [] events_;
}


std::vector<Channel*> Epoll::poll(int timeout)
{
    std::vector<Channel*> activeChannels;
    int nfds;
    while(true){
        nfds = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
        if(nfds == -1){
            if(errno == EINTR) continue;
            else errif(nfds == -1, "epoll wait error");
        }
        break;
    }
    
    for(int i = 0; i < nfds; ++i){
        Channel *ch = (Channel*)events_[i].data.ptr;
        ch->setReady(events_[i].events);
        activeChannels.push_back(ch);
    }
    return activeChannels;
}

void Epoll::updateChannel(Channel *channel)
{
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if(!channel->getInEpoll())
    {
        errif(epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->setInEpoll();
        // debug("Epoll: add Channel to epoll tree success, the Channel's fd is: ", fd);
    } 
    else
    {
        errif(epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
        // debug("Epoll: modify Channel in epoll tree success, the Channel's fd is: ", fd);
    }
}

void Epoll::deleteChannel(Channel* channel)
{
    int fd = channel->getFd();
    errif(epoll_ctl(epfd_,EPOLL_CTL_DEL,fd,nullptr) == -1, "epoll delete error");
    channel->setInEpoll(false);
}