#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include "Acceptor.h"
#include "Connection.h"

Server::Server(EventLoop* _lp) : lp(_lp) {
    acceptor = new Acceptor(lp);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->set_conn_callback(cb);
}

Server::~Server() {
    delete acceptor;
}

void Server::newConnection(Socket* serv_sockfd) {
    Connection* tcp_connection = new Connection(lp, serv_sockfd);
    connections[serv_sockfd->get_fd()] = tcp_connection;
    std::function<void(Socket*)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    tcp_connection->set_delete_connection_callback(cb);
}

void Server::deleteConnection(Socket* serv_sockfd) {
    connections.erase(serv_sockfd->get_fd());
}



