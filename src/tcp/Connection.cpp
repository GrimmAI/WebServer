#include "Connection.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include <cstring>
#include <unistd.h>


Connection::Connection(EventLoop* _lp, Socket* _serv_sockfd) : lp(_lp), serv_sockfd(_serv_sockfd) {
    clnt_addr = new InetAddress();
    clnt_sockfd = serv_sockfd->accept(clnt_addr);
    clnt_sockfd->setnonblocking();
    Channel* clnt_Channel = new Channel(lp, clnt_sockfd->get_fd());
    clnt_Channel->enableReading();
    std::function<void()> callback = std::bind(&Connection::echo, this, clnt_sockfd);
    clnt_Channel->set_event_callback(callback);
}

Connection::~Connection() {
    cb(clnt_sockfd);
    delete clnt_addr;
}

void Connection::echo(Socket* clnt_sockfd) {
    char buf[1024];
    while (true) {
        memset(buf, 0, sizeof buf);
        ssize_t read_bytes = read(clnt_sockfd->get_fd(), buf, sizeof buf);
        if (read_bytes > 0) {
            printf("message from client fd %d: %s\n", clnt_sockfd->get_fd(), buf);
            write(clnt_sockfd->get_fd(), buf, sizeof buf);
        } else if (read_bytes == -1 && errno == EINTR) {
            printf("continue reading");
            continue;
        } else if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // printf("finish reading once, errno: %d\n", errno);
            break;
        } else if (read_bytes == 0) {
            printf("EOF, client fd %d disconnected\n", clnt_sockfd->get_fd());
            close(clnt_sockfd->get_fd());   //关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}


void Connection::set_delete_connection_callback(std::function<void(Socket*)> func) {
    cb = func;
}