#pragma once
#include <arpa/inet.h>

class InetAddress;
class Socket {
private:
    int sockfd;
public:
    Socket();
    Socket(int fd);
    ~Socket();

    void bind(InetAddress* serv_addr);
    void listen();
    void setnonblocking();
    Socket* accept(InetAddress* clnt_addr);
    int get_fd();
};