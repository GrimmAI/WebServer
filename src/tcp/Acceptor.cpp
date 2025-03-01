#include "Acceptor.h"
#include "Channel.h"
#include "Server.h"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>


Acceptor::Acceptor(EventLoop* _lp, std::string ip, int port) : lp(_lp) {
    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sockfd == -1) {
        std::cout << "socket create error" << std::endl;
        exit(-1);
    }
    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    serv_addr.sin_port = htons(port);
    if (::bind(serv_sockfd, (sockaddr*)&serv_addr, sizeof serv_addr) == -1) {
        std::cout << "bind error" << std::endl;
        exit(-1);
    }
    if (::listen(serv_sockfd, SOMAXCONN)) {
        std::cout << "listen error" << std::endl;
        exit(-1);
    }
    fcntl(serv_sockfd, F_SETFL, fcntl(serv_sockfd, F_GETFL) | O_NONBLOCK);

    Channel* serv_Channel = new Channel(lp, serv_sockfd);
    serv_Channel->enableReading();
    std::function<void()> callback = std::bind(&Acceptor::acceptConnection, this);
    serv_Channel->set_event_callback(callback);
}

Acceptor::~Acceptor() {
    if (serv_sockfd != -1)
        close(serv_sockfd);
}

void Acceptor::acceptConnection() {
    cb(serv_sockfd);
}

void Acceptor::set_conn_callback(std::function<void(int)> func) {
    cb = func;
}


