#pragma once
#include <functional>
#include <arpa/inet.h>
#include <string>

class EventLoop;
class Acceptor {
private:
    int serv_sockfd;
    struct sockaddr_in serv_addr;
    EventLoop* lp;
    std::function<void(int)> cb;

public:
    Acceptor(EventLoop*, std::string, int);
    ~Acceptor();
    void set_conn_callback(std::function<void(int)>);
    void acceptConnection();
};