#include "net_src/Connection.h"
#include "net_src/Socket.h"
#include <iostream>
#include "byteSerialize/byteSerializer.h"
#include "net_src/Buffer.h"

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

int main() {
  Socket *sock = new Socket();
  sock->Connect("127.0.0.1", 8888);

  Connection *conn = new Connection(nullptr, sock);

  ByteStream stream;
  client_struct client_1;
  
  client_1.key = 1;
  client_1.name = "client1";
  while(true){
    std::cout<<"输入对话"<<"\n";
    std::string str;
    std::getline(std::cin, str);
    client_1.msg = str;
    std::cout<<"输入命令"<<"\n";
    str.clear();
    std::getline(std::cin, str);
    client_1.Command = str;

    Serlize(stream,client_1);
    
    conn->SetSendBuffer((const char*)stream.get_buffer().data(),stream.get_buffer().size());
    stream.Clear();

    conn->Write();
    conn->Read();
    std::cout << "Message from server: " << conn->ReadBuffer() << std::endl;
  }
  

  delete conn;
  return 0;
}