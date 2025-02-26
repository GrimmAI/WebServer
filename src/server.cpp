#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS 100


inline void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

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
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket create error");
        exit(-1);
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    if (bind(sockfd, (sockaddr*)&serv_addr, sizeof serv_addr) == -1) {
        printf("bind error");
        exit(-1);
    }

    if (listen(sockfd, SOMAXCONN)) {
        printf("listen error");
        exit(-1);
    }

    struct sockaddr_in clnt_addr;
    memset(&clnt_addr, 0, sizeof clnt_addr);
    socklen_t clnt_addr_len = sizeof(clnt_addr);

    int epfd = epoll_create1(0);
    struct epoll_event events[MAX_EVENTS], ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == sockfd) {
                int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
                ev.data.fd = clnt_sockfd;
                ev.events = EPOLLIN | EPOLLET;
                setnonblocking(clnt_sockfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev);
            } else if (events[i].events & EPOLLIN) {
                handleEvent(events[i].data.fd);
            }
        }
    }

    close(sockfd);
    return 0;
}