#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <cerrno>

Socket::Socket() : fd_(-1)
{
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd_ == -1, "socket create error");
}
Socket::Socket(int fd) : fd_(fd)
{
    errif(fd_ == -1, "socket create error");
}

Socket::~Socket()
{
    if(fd_ != -1)
    {
        close(fd_);
        fd_ = -1;
    }
}

void Socket::bind(InetAddress *addr)
{
    struct sockaddr_in tmp_addr = addr->GetAddr();
    errif(::bind(fd_, (sockaddr*)&tmp_addr, sizeof(tmp_addr)) == -1, "socket bind error");
}

void Socket::listen()
{
    errif(::listen(fd_, SOMAXCONN) == -1, "socket listen error");
}
void Socket::setnonblocking()
{
    fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK);
}
bool Socket::Isnonblocking()
{
    return (fcntl(fd_,F_GETFL) & O_NONBLOCK) != 0;
}

int Socket::accept(InetAddress *addr)
{
    int clnt_sockfd = -1;
    struct sockaddr_in tmp_addr {};
    socklen_t addr_len = sizeof(tmp_addr);
    if(Isnonblocking())
    {
        while(true)
        {
            clnt_sockfd = ::accept(fd_, (sockaddr*)&tmp_addr, &addr_len);
            if(clnt_sockfd == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) continue;
            if(clnt_sockfd == -1) errif(true, "socket accept error");
            else break;
        }
    }
    else
    {
        clnt_sockfd = ::accept(fd_, (sockaddr*)&tmp_addr, &addr_len);
        errif(clnt_sockfd == -1, "socket accept error");
    }
    addr->SetAddr(tmp_addr);
    return clnt_sockfd;
}

int Socket::getFd()
{
    return fd_;
}

void Socket::Connect(InetAddress *addr)
{
    struct sockaddr_in tmp_addr = addr->GetAddr();
    if(fcntl(fd_,F_GETFL) & O_NONBLOCK)
    {
        while (true)
        {
            int ret = connect(fd_,(sockaddr*)&tmp_addr,sizeof(tmp_addr));
            if(ret == 0) break;
            if(ret == -1 && (errno == EINPROGRESS)) continue;
            if(ret == -1) errif(true,"socket connect error");
        }
        
    }
    else errif(connect(fd_, (sockaddr *)&tmp_addr, sizeof(tmp_addr)) == -1, "socket connect error");
}

void Socket::Connect(const char *ip,uint16_t port)
{
    InetAddress *addr = new InetAddress(ip,port);
    Connect(addr);
    delete addr;
}