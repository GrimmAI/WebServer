#include "InetAddress.h"
#include <cstring>

InetAddress::InetAddress() : addr_len(sizeof sock_addr) {
    memset(&sock_addr, 0, sizeof sock_addr);
}

InetAddress::InetAddress(std::string ip, int port) : addr_len(sizeof sock_addr) {
    memset(&sock_addr, 0, sizeof sock_addr);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    sock_addr.sin_port = htons(port);

}

InetAddress::~InetAddress() {

}