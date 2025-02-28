#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Server.h"


Acceptor::Acceptor(EventLoop* _lp) : lp(_lp) {
    sockfd = new Socket();
    serv_addr = new InetAddress("127.0.0.1", 8888);
    sockfd->bind(serv_addr);
    sockfd->listen();
    sockfd->setnonblocking();

    Channel* serv_Channel = new Channel(lp, sockfd->get_fd());
    serv_Channel->enableReading();
    std::function<void()> callback = std::bind(&Acceptor::acceptConnection, this);
    serv_Channel->set_event_callback(callback);
}

Acceptor::~Acceptor() {
    delete sockfd;
    delete serv_addr;
}

void Acceptor::acceptConnection() {
    cb(sockfd);
}

void Acceptor::set_conn_callback(std::function<void(Socket*)> func) {
    cb = func;
}


