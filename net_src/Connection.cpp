#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <utility>
#include "Buffer.h"
#include "util.h"
#include <cstring>

#define READ_BUFFER 1024

Connection::Connection(EventLoop *loop,Socket *sock):loop_(loop),sock_(sock)
{
    if(loop_ != nullptr)
    {
        channel_ = new Channel(loop_,sock->getFd());
        channel_->enableReading();
        channel_->useET();
    }
    readBuffer_ = new Buffer();
    sendBuffer_ = new Buffer();
    state_ = State::Connected;
}
Connection::~Connection()
{
    if(loop_ != nullptr)
    {
        delete channel_;
    }
    delete sock_;
    delete readBuffer_;
    delete sendBuffer_;
}

void Connection::Read()
{
    ASSERT(state_ == State::Connected,"connection state is disConnected!");
    readBuffer_->clear();
    if(sock_->Isnonblocking()) ReadNonBlocking();
    else ReadBlocking();
}

void Connection::Write()
{
    ASSERT(state_ == State::Connected,"connection state is disConnected!");
    if(sock_->Isnonblocking()) WriteNonBlocking();
    else WriteBlocking();
    sendBuffer_->clear();
}

void Connection::ReadNonBlocking()
{
    int sockfd = sock_->getFd();
    char buf[1024];
    while(true)
    {
        memset(buf,0,sizeof(buf));
        ssize_t bytes_read = read(sockfd,buf,sizeof(buf));
        if(bytes_read >0)
        {
            readBuffer_->append(buf,bytes_read);
        }
        else if(bytes_read == -1 &&
                ((errno == EAGAIN) || (errno == EWOULDBLOCK))) break;
        
        else if(bytes_read == 0)
        {
            std::cout<<"read EOF, client fd "<<sockfd<<" disconnected"<<"\n";
            state_ = State::Closed;
            break;
        }        
        else
        {
            std::cout<<"other error on client fd "<<sockfd<<"\n";
            state_ = State::Closed;
            break;
        }
    }
}

void Connection::WriteNonBlocking()
{
    int sockfd = sock_->getFd();
    char buf[sendBuffer_->size()];
    memcpy(buf,sendBuffer_->c_str(),sendBuffer_->size());
    int data_size = sendBuffer_->size();
    int data_left = data_size;
    while(data_left > 0)
    {
        ssize_t bytes_write = write(sockfd, buf + data_size - data_left,data_left);
        if(bytes_write == -1 && errno == EAGAIN) break;
        if(bytes_write == -1 && errno == EINTR) 
        {
            std::cout<<"continue writing\n";
            continue;
        }
        if(bytes_write == -1)
        {
            std::cout<<"other error on client fd "<<sockfd<<"\n";
            state_ = State::Closed;
            break;
        }
        data_left -= bytes_write;
    }
}

void Connection::ReadBlocking()
{
    int sockfd = sock_->getFd();
    unsigned int rcv_size = 0;
    socklen_t len = sizeof(rcv_size);
    getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&rcv_size,&len);
    char buf[rcv_size];
    ssize_t bytes_read = read(sockfd,buf,sizeof(buf));
    if(bytes_read >0)
    {
        readBuffer_->append(buf,bytes_read);
    }
    else if(bytes_read == 0)
    {
        std::cout<<"read EOF,blocking client fd "<<sockfd<<" disconnected"<<"\n";
        state_ = State::Closed;
    }        
    else if(bytes_read == -1)
    {
        std::cout<<"other error on client fd "<<sockfd<<"\n";
        state_ = State::Closed;
    }
}

void Connection::WriteBlocking()
{
    int sockfd = sock_->getFd();
    ssize_t bytes_write = write(sockfd,sendBuffer_->c_str(),sendBuffer_->size());
    if(bytes_write == -1)
    {
        std::cout<<"other error on blocking client fd "<<sockfd<<"\n";
        state_ = State::Closed;
    }
}

void Connection::Close()
{
    deleteConnectionCallback_(sock_);
}

Connection::State Connection::GetState(){return state_;}

void Connection::SetSendBuffer(const char* str){sendBuffer_->SetBuf(str);}

void Connection::SetSendBuffer(const char* str,size_t len){sendBuffer_->SetBuf(str,len);}

Buffer *Connection::GetReadBuffer() {return readBuffer_;}

const char* Connection::ReadBuffer(){return readBuffer_->c_str();}

Buffer *Connection::GetSendBuffer(){return sendBuffer_;}

const char* Connection::SendBuffer(){return sendBuffer_->c_str();}


void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> const &cb)
{
    deleteConnectionCallback_ = cb;
}

void Connection::setOnConnectionCallback(std::function<void(Connection*)> const &cb)
{
    onConnectCallback_ = cb;
    //channel_->setReadCallback([this](){onConnectCallback_(this);});
}

void Connection::setOnMessageCallback(std::function<void(Connection *)> const &cb) 
{
    onMessageCallback_ = cb;
    std::function<void()> bus = std::bind(&Connection::Business, this);
    channel_->setReadCallback(bus);
}

void Connection::GetlineSendBuffer(){sendBuffer_->getline();}

Socket *Connection::GetSocket(){return sock_;}

void Connection::send(std::string msg){
    SetSendBuffer(msg.c_str());
    Write();
}

void Connection::Business(){
  Read();
  if(state_ == Connected){
    onMessageCallback_(this);
  }
}