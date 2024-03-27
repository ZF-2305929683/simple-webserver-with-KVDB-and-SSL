#pragma once
#include <functional>
#include "Macros.h"
class EventLoop;
class Socket;
class Channel;
class Buffer;
class Connection
{
public:
    enum State
    {
        Invalid = 1,
        Handshaking,
        Connected,
        Closed,
        Failed,
    };

    Connection(EventLoop *_loop,Socket *_sock);
    ~Connection();

    DISALLOW_COPY_AND_MOVE(Connection);

    void Read();
    void Write();
    void setDeleteConnectionCallback(std::function<void(Socket*)> const &cb);
    void setOnConnectionCallback(std::function<void(Connection*)> const &cb);
    void setOnMessageCallback(std::function<void(Connection*)> const &cb);
    void Business();

    void send(std::string msg);

    State GetState();
    void Close();
    void SetSendBuffer(const char *str);
    Buffer *GetReadBuffer();
    const char* ReadBuffer();
    Buffer *GetSendBuffer();
    const char* SendBuffer();
    void GetlineSendBuffer();
    Socket* GetSocket();

    void OnConnect(std::function<void()> fn);

private:
    EventLoop *loop_;
    Socket *sock_;
    Channel *channel_;
    State state_{State::Invalid};
    std::function<void(Socket*)> deleteConnectionCallback_;

    std::function<void(Connection*)> onConnectCallback_;
    std::function<void(Connection*)> onMessageCallback_;
    Buffer *readBuffer_;
    Buffer *sendBuffer_;

    void ReadNonBlocking();
    void WriteNonBlocking();
    void ReadBlocking();
    void WriteBlocking();
};


