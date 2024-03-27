#include "net_src/Connection.h"
#include "net_src/Socket.h"
#include <iostream>

int main() {
  Socket *sock = new Socket();
  sock->Connect("127.0.0.1", 8888);

  Connection *conn = new Connection(nullptr, sock);
  /*while(true){
    conn->Read();
    std::cout << "Message from server: " << conn->ReadBuffer() << std::endl;
  }*/
  //conn->Read();
  conn->SetSendBuffer("Hello server!");
  conn->Write();
  while(true){
    std::cout<<"输入对话"<<"\n";
    std::string str;
    std::cin>>str;
    conn->SetSendBuffer(str.c_str());
    conn->Write();
    conn->Read();
    std::cout << "Message from server: " << conn->ReadBuffer() << std::endl;
  }
  

  delete conn;
  return 0;
}