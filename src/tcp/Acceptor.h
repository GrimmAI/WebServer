#pragma once
#include <functional>
#include <arpa/inet.h>
#include <string>
#include <memory>

class EventLoop;
class Channel;
class Acceptor {
private:
    int serv_sockfd_;
    std::unique_ptr<Channel> serv_Channel_;
    struct sockaddr_in serv_addr_;
    std::shared_ptr<EventLoop> lp_;
    std::function<void(int)> conn_cb_;

public:
    Acceptor(std::shared_ptr<EventLoop>, std::string, int);
    ~Acceptor();
    void SetConnCallback(std::function<void(int)> &&);
    void AcceptConnection();
};