#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Server.h"
#include "Acceptor.h"
#include "Connection.h"
#include <functional>
#include <iostream>
#include <errno.h>
#include <thread>
#include "ThreadPool.h"
#include "util.h"



Server::Server(EventLoop* loop):mainReactor_(loop),acceptor_(nullptr)
{
    acceptor_ = new Accepter(mainReactor_);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection,this,std::placeholders::_1);
    acceptor_->setNewConnectionCallback(cb);

    int size = 2;
    thpool_ = new ThreadPool(size);
    for(int i = 0;i<size;++i)
    {
        subReactors_.push_back(new EventLoop());
    }

    for(int i = 0;i<size;++i)
    {
        std::function<void()> sub_loop = std::bind(&EventLoop::loop,subReactors_[i]);
        thpool_->add(std::move(sub_loop));
    }
}

Server::~Server()
{
    for(EventLoop *each : subReactors_){
        delete each;
    }
    delete acceptor_;
    delete thpool_;
}



void Server::newConnection(Socket *sock)
{
    errif(sock->getFd() == -1, "new connection error");
    uint64_t random = sock->getFd() % subReactors_.size();
    Connection *conn = new Connection(subReactors_[random], sock);
    std::function<void(Socket *)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    conn->setOnMessageCallback(onMessageCallback_);
    connections_[sock->getFd()] = conn;
    if(NewConnectCallback_) NewConnectCallback_(conn);
}

void Server::deleteConnection(Socket *sock)
{
    int sockfd = sock->getFd();
    auto it = connections_.find(sockfd);
    if(it != connections_.end())
    {
        Connection *conn = connections_[sockfd];
        connections_.erase(sockfd);
        delete conn;
        conn = nullptr;
    }
    
}

void Server::OnConnect(std::function<void(Connection*)> fn){onConnectCallback_ = std::move(fn);}

void Server::NewConnect(std::function<void(Connection*)> fn){NewConnectCallback_ = std::move(fn);}

void Server::OnMessage(std::function<void(Connection*)> fn){onMessageCallback_ = std::move(fn);}