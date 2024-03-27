#include "net_src/Server.h"
#include "net_src/EventLoop.h"
#include "net_src/Connection.h"
#include "net_src/Socket.h"
#include <memory>
#include <map>
#include <iostream>

int main()
{
    std::map<int, Connection *> clients;
    EventLoop *loop = new EventLoop();

    Server *server = new Server(loop);

    server->NewConnect(
        [&](Connection *conn) {
        int clnt_fd = conn->GetSocket()->getFd();
        std::cout << "New connection fd: " << clnt_fd << std::endl;
        clients[clnt_fd] = conn;
          for(auto &each : clients){
          Connection *client = each.second;
          client->send(conn->ReadBuffer());
        }
    });

    server->OnMessage(
      [&](Connection *conn){
        std::cout << "Message from client " << conn->ReadBuffer() << std::endl;
        std::string str = "hahahahahah";
        for(auto &each : clients){
          Connection *client = each.second;
          client->SetSendBuffer(str.c_str());
          client->Write();
        }
      }
    );
    loop->loop();
    return 0;
}

