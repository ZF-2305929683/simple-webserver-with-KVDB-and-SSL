#pragma once
#include <map>
#include "Macros.h"
#include <vector>
#include <functional>
class Accepter;
class EventLoop;
class Socket;
class Connection;
class ThreadPool;
class Server
{
private:
    EventLoop *mainReactor_;
    Accepter *acceptor_;
    std::map<int,Connection*> connections_;
    std::vector<EventLoop*> subReactors_;
    ThreadPool *thpool_;
    std::function<void(Connection*)> onConnectCallback_;
    std::function<void(Connection*)> onMessageCallback_;
    std::function<void(Connection*)> NewConnectCallback_;
public:
    explicit Server(EventLoop *);
    ~Server();

    DISALLOW_COPY_AND_MOVE(Server);
    void newConnection(Socket *sock);
    void deleteConnection(Socket *sock);
    void OnConnect(std::function<void(Connection*)> fn);
    void OnMessage(std::function<void(Connection *)> fn);
    void NewConnect(std::function<void(Connection *)> fn);
};
