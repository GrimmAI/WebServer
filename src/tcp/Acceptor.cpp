#include "Acceptor.h"
#include "Channel.h"
#include "TcpServer.h"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

Acceptor::Acceptor(std::shared_ptr<EventLoop> lp, std::string ip, int port) : lp_(lp) {
    serv_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sockfd_ == -1) {
        std::cout << "socket create error" << std::endl;
        exit(-1);
    }
    memset(&serv_addr_, 0, sizeof serv_addr_);
    serv_addr_.sin_family = AF_INET;
    serv_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    serv_addr_.sin_port = htons(port);
    if (::bind(serv_sockfd_, (sockaddr *)&serv_addr_, sizeof serv_addr_) == -1) {
        std::cout << "bind error" << std::endl;
        exit(-1);
    }
    if (::listen(serv_sockfd_, SOMAXCONN)) {
        std::cout << "listen error" << std::endl;
        exit(-1);
    }
    fcntl(serv_sockfd_, F_SETFL, fcntl(serv_sockfd_, F_GETFL) | O_NONBLOCK);

    serv_Channel_ = std::make_unique<Channel>(lp, serv_sockfd_);
    serv_Channel_->EnableReading();
    std::function<void()> callback = std::bind(&Acceptor::AcceptConnection, this);
    serv_Channel_->SetEventCallback(std::move(callback));
}

Acceptor::~Acceptor() {
    if (serv_sockfd_ != -1)
        close(serv_sockfd_);
}

void Acceptor::AcceptConnection() {
    conn_cb_(serv_sockfd_);
}

void Acceptor::SetConnCallback(std::function<void(int)> &&func) {
    conn_cb_ = std::move(func);
}


