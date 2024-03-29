#include "net_src/Server.h"
#include "net_src/EventLoop.h"
#include "net_src/Connection.h"
#include "net_src/Socket.h"
#include <memory>
#include <map>
#include <iostream>
#include "byteSerialize/byteSerializer.h"
#include "net_src/Buffer.h"


struct client_struct
{
    int key;
    std::string name;
    std::string msg;
}; 

template<>
struct TypeInfo<client_struct> :TypeInfoBase<client_struct>
{
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
        Field {register("key"), &Type::key},
        Field {register("name"), &Type::name},
        Field {register("msg"), &Type::msg},
    };
};



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

    ByteStream stream;

    server->OnMessage(
      [&](Connection *conn){

       
        ByteStream stream(conn->ReadBuffer(),conn->GetReadBuffer()->size());
        client_struct client;
        Deserlize(stream,client);

        std::cout<<"\n";
        std::cout << "Message from client: " << client.key <<"\n"<<"Name is: "<< client.name <<"\n"<<"Message is: " << client.msg <<"\n";
        std::cout<<"\n";
        
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

