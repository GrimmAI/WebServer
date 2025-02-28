#pragma once
#include <functional>
class EventLoop;
class Socket;
class InetAddress;
class Buffer;
class Connection {
private:
    EventLoop* lp;
    Socket* clnt_sockfd;
    InetAddress* clnt_addr;
    Socket* serv_sockfd;
    std::function<void(Socket*)> cb;
    Buffer* readBuffer;
public:
    Connection(EventLoop*, Socket*);
    ~Connection();
    void set_delete_connection_callback(std::function<void(Socket*)>);
    void echo(Socket*);
};