#include "Server.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "EventLoop.h"

Server::Server(std::string _ip, int _port) : idx(0), ip(_ip), port(_port) {
    main_reactor = std::make_unique<EventLoop>();
    acceptor = std::make_unique<Acceptor>(main_reactor, ip, port);

    std::function<void(int)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->set_conn_callback(cb);


    int thread_num = 10;
    thread_pool = std::make_unique<ThreadPool>(thread_num);

    for (int i = 0; i < thread_num; i++) {
        sub_reactor.push_back(std::make_unique<EventLoop>());
        std::function<void()> func = std::bind(&EventLoop::loop, sub_reactor[i]);
        thread_pool->enqueue(func);
    }

}

Server::~Server() {

}



void Server::newConnection(int serv_sockfd) {
    Connection* tcp_connection = new Connection(sub_reactor[idx % sub_reactor.size()].get(), serv_sockfd, ip, port);
    tcp_connection->set_handle_message_callback(handle_message_callback);
    connections[serv_sockfd] = tcp_connection;
    std::function<void(int)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    tcp_connection->set_delete_connection_callback(cb);
    idx++;
}

void Server::deleteConnection(int serv_sockfd) {
    Connection* conn = connections[serv_sockfd];
    connections.erase(serv_sockfd);
    delete conn;
}


void Server::set_handle_message_callback(std::function<void(int)> func) {
    handle_message_callback = func;
}

void Server::start() {
    main_reactor->loop();
}

