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

#define MAX_EVENTS 100


inline void handleEvent(int clnt_sockfd) {
    char buf[1024];
    while (true) {
        memset(buf, 0, sizeof buf);
        ssize_t read_bytes = read(clnt_sockfd, buf, sizeof buf);
        if (read_bytes > 0) {
            printf("message from client fd %d: %s\n", clnt_sockfd, buf);
            write(clnt_sockfd, buf, sizeof buf);
        } else if (read_bytes == -1 && errno == EINTR) {
            printf("continue reading");
            continue;
        } else if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            printf("finish reading once, errno: %d\n", errno);
            break;
        } else if (read_bytes == 0) {
            printf("EOF, client fd %d disconnected\n", clnt_sockfd);
            close(clnt_sockfd);   //关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}

int main() {
    Socket* sockfd = new Socket();
    InetAddress* serv_addr = new InetAddress("127.0.0.1", 8888);
    sockfd->bind(serv_addr);
    sockfd->listen();

    InetAddress* clnt_addr = new InetAddress();

    Epoll* epfd = new Epoll();
    epfd->addfd(sockfd->get_fd(), EPOLLIN);

    while (true) {
        std::vector<epoll_event> events = epfd->poll();
        for (auto &ev: events) {
            if (ev.data.fd == sockfd->get_fd()) {
                Socket* clnt_sockfd = sockfd->accept(clnt_addr);
                clnt_sockfd->setnonblocking();
                epfd->addfd(clnt_sockfd->get_fd(), EPOLLIN | EPOLLET);
            } else if (ev.events & EPOLLIN) {
                handleEvent(ev.data.fd);
            }
        }
    }

    delete sockfd;
    delete epfd;
    return 0;
}