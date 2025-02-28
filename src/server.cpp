#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "tcp/Socket.h"
#include "tcp/InetAddress.h"
#include "tcp/Epoll.h"
#include "tcp/Channel.h"
#include "tcp/EventLoop.h"
#include "tcp/Server.h"

int main() {
    EventLoop* loop = new EventLoop();
    Server* server = new Server(loop);

    loop->loop();

    delete loop;
    delete server;
    return 0;
}