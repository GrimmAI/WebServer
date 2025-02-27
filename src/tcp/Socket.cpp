#include "Socket.h"
#include "InetAddress.h"
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>


Socket::Socket() : sockfd(-1) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cout << "socket create error" << std::endl;
        exit(-1);
    }
}

Socket::Socket(int fd) : sockfd(fd) {
    if (sockfd == -1) {
        std::cout << "socket create error" << std::endl;
        exit(-1);
    }
}

Socket::~Socket() {
    if (sockfd != -1)
        close(sockfd);
}

void Socket::bind(InetAddress* serv_addr) {
    if (::bind(sockfd, (sockaddr*)&serv_addr->sock_addr, sizeof serv_addr->sock_addr) == -1) {
        std::cout << "bind error" << std::endl;
        exit(-1);
    }
}

void Socket::listen() {
    if (::listen(sockfd, SOMAXCONN)) {
        std::cout << "listen error" << std::endl;
        exit(-1);
    }
}

void Socket::setnonblocking() {
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
}

Socket* Socket::accept(InetAddress* clnt_addr) {
    int clnt_fd = ::accept(sockfd, (sockaddr*)&clnt_addr->sock_addr, &clnt_addr->addr_len);
    if (clnt_fd == -1) {
        std::cout << "accept client error" << std::endl;
        exit(-1);
    }
    Socket* clnt_sock = new Socket(clnt_fd);
    return clnt_sock;
}

int Socket::get_fd() {
    return sockfd;
}




