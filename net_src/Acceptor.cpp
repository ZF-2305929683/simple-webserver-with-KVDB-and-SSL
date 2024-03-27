#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Server.h"
#include <iostream>
#include <errno.h>


Accepter::Accepter(EventLoop * loop):loop_(loop)
{
    sock_ = new Socket();
    addr_ = new InetAddress("127.0.0.1",8888);
    sock_->bind(addr_);
    sock_->listen();
    
    acceptChannel_ = new Channel(loop,sock_->getFd());

    std::function<void()> cb = std::bind(&Accepter::acceptConnection,this);
    acceptChannel_->setReadCallback(cb);
    acceptChannel_->enableReading();
}

Accepter::~Accepter()
{
    delete sock_;
    delete addr_;
    delete acceptChannel_;
}

void Accepter::acceptConnection()
{
    InetAddress *client_addr = new InetAddress();
    Socket * client_sock = new Socket(sock_->accept(client_addr));
    std::cout<<"new client fd "<<client_sock->getFd()<<"IP"
                <<inet_ntoa(client_addr->GetAddr().sin_addr)<<"Port"
                <<ntohs(client_addr->GetAddr().sin_port)<<std::endl;

    client_sock->setnonblocking();
    newConnectionCallback_(client_sock);
    delete client_addr;
}

void Accepter::setNewConnectionCallback(std::function<void(Socket*)> _cb)
{
    newConnectionCallback_ = _cb;
}