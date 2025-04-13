#include "TcpServer.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "EventLoop.h"
#include <iostream>

TcpServer::TcpServer(std::string ip, int port) : sub_reactor_idx_(0), server_ip_(ip), server_port_(port) {
    main_reactor_ = std::make_shared<EventLoop>();
    acceptor_ = std::make_unique<Acceptor>(main_reactor_, server_ip_, server_port_);

    std::function<void(int)> cb = std::bind(&TcpServer::NewConnection, this, std::placeholders::_1);
    acceptor_->SetConnCallback(std::move(cb));

    int thread_num = 10;
    thread_pool_ = std::make_unique<ThreadPool>(thread_num);

    for (int i = 0; i < thread_num; i++) {
        sub_reactor_.push_back(std::make_shared<EventLoop>());
        std::function<void()> func = std::bind(&EventLoop::Loop, sub_reactor_[i].get());
        thread_pool_->enqueue(func);
    }
}

TcpServer::~TcpServer() {

}

void TcpServer::NewConnection(int serv_sockfd) {
    auto new_conn = std::make_shared<Connection>(sub_reactor_[sub_reactor_idx_ % sub_reactor_.size()], 
                                                             serv_sockfd, server_ip_, server_port_);
    connections_[new_conn->clnt_sockfd_] = new_conn;
    connections_[new_conn->clnt_sockfd_]->SetHandleMessageCallback(std::move(handle_message_callback_));
    std::function<void(int)> cb = std::bind(&TcpServer::DeleteConnection, this, std::placeholders::_1);
    connections_[new_conn->clnt_sockfd_]->SetDeleteConnectionCallback(std::move(cb));
    sub_reactor_idx_++;
}

void TcpServer::DeleteConnection(int clnt_sockfd) {
    connections_[clnt_sockfd]->connection_state_ = Connection::ConnectionState::Disconected;
    connections_[clnt_sockfd]->lp_->DeleteChannel(connections_[clnt_sockfd]->clnt_Channel_.get());
    connections_.erase(clnt_sockfd);
}

void TcpServer::SetHandleMessageCallback(std::function<void(Connection*)> &&func) {
    handle_message_callback_ = std::move(func);
}

void TcpServer::Start() {
    main_reactor_->Loop();
}

