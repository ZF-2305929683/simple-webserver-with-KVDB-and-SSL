#include "net_src/Server.h"
#include "net_src/EventLoop.h"
#include "net_src/Connection.h"
#include "net_src/Socket.h"
#include <memory>
#include <map>
#include <iostream>
#include "byteSerialize/byteSerializer.h"
#include "net_src/Buffer.h"
#include "simple KVdb/GlobalCommandList.h"
#include "simple KVdb/skiplist.h"
#include "Singleton.h"
#include "SSL/SSL.h"


struct client_struct
{
    int key;
    std::string name;
    std::string msg;
    std::string Command;
}; 


template<>
struct TypeInfo<client_struct> :TypeInfoBase<client_struct>
{
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
        Field {register("key"), &Type::key},
        Field {register("name"), &Type::name},
        Field {register("msg"), &Type::msg},
        Field {register("Command"), &Type::Command},
    };
};





int main()
{
    std::map<int, Connection *> clients;
    EventLoop *loop = new EventLoop();

    SkipList<std::string,std::string> &KV_DB = Singleton<SkipList<std::string,std::string>>::Instance(3);
    GlobalCommandList &global_list = Singleton<GlobalCommandList>::Instance();

    Server *server = new Server(loop);

    simple_SSL Server(NetworkType::Server);

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
        std::cout << "Message from client: " << client.key <<"\n"<<"Name is: "<< client.name <<"\n"<<"Message is: " << client.msg <<"\n""Command is: " << client.Command <<"\n";
        std::cout<<"\n";

        global_list.Insert(client.Command);

        std::string str = "hahahahahah";

        for(auto &each : clients){
          Connection *client = each.second;
          client->SetSendBuffer(str.c_str());
          client->Write();
        }
      }
    );

    std::thread readAnd_Do([&]() {
    while (true) {
            std::string Command;
            std::string key;
            std::string value;
            
    
            global_list.CommandParsing(Command,key,value);
    
            if(Command == "Insert") KV_DB.insert_element(key,value);
            else if(Command == "display") KV_DB.display_list();
            else{
              std::cout<<"命令错误"<<"\n";
            }

        }
    });
    loop->loop();
    if(readAnd_Do.joinable()) readAnd_Do.join();
    return 0;
}

