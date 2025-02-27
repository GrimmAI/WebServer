#pragma once
#include <arpa/inet.h>
#include <string>

class InetAddress {
public:
    struct sockaddr_in sock_addr;
    socklen_t addr_len;
    InetAddress();
    InetAddress(std::string, int);
    ~InetAddress();
};