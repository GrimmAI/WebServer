#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "tcp/Epoll.h"
#include "tcp/Channel.h"
#include "tcp/EventLoop.h"
#include "tcp/Server.h"
#include "tcp/Buffer.h"
#include <iostream>
#include "tcp/Connection.h"


int main(int argc, char *argv[]) {
    
    Server* server = new Server("127.0.0.1", 8888);

    server->set_handle_message_callback([](Connection* conn) {
        conn->read();
        std::cout << "Message from client "  << conn->get_read_buffer() << std::endl;
        conn->send(conn->get_read_buffer()); 
    });

    server->start();

    delete server;
    return 0;
}