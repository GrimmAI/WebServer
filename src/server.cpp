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


    int epfd = epoll_create1(0);
    struct epoll_event events[MAX_EVENTS], ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd->get_fd();
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd->get_fd(), &ev);

    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == sockfd->get_fd()) {
                auto clnt_sockfd = sockfd->accept(clnt_addr);
                ev.data.fd = clnt_sockfd->get_fd();
                ev.events = EPOLLIN | EPOLLET;
                clnt_sockfd->setnonblocking();
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd->get_fd(), &ev);
            } else if (events[i].events & EPOLLIN) {
                handleEvent(events[i].data.fd);
            }
        }
    }

    delete sockfd;
    return 0;
}