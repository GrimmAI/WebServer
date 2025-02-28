#pragma once
#include <functional>

class Socket;
class InetAddress;
class EventLoop;
class Acceptor {
private:
    Socket* sockfd;
    InetAddress* serv_addr;
    EventLoop* lp;
    std::function<void(Socket*)> cb;

public:
    Acceptor(EventLoop*);
    ~Acceptor();
    void set_conn_callback(std::function<void(Socket*)>);
    void acceptConnection();
};