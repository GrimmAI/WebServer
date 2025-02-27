#include "tcp/Socket.h"
#include "tcp/InetAddress.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int main() {
    Socket* sockfd = new Socket();
    InetAddress* serv_addr = new InetAddress("127.0.0.1", 8888);

    sockfd->connect(serv_addr);

    while (true) {
        char buf[1024];
        memset(buf, 0, sizeof buf);
        scanf("%s", buf);
        ssize_t write_bytes = write(sockfd->get_fd(), buf, sizeof buf);
        if (write_bytes == -1) {
            printf("socket already disconnected, can't write any more\n");
            break;
        }
        memset(buf, 0, sizeof buf);
        ssize_t read_bytes = read(sockfd->get_fd(), buf, sizeof buf);
        if (read_bytes > 0) {
            printf("message from server: %s\n", buf);
        } else if (read_bytes == 0) {
            printf("server socket disconnected!\n");
            break;
        } else if (read_bytes == -1) {
            close(sockfd->get_fd());
            printf("socket read error");
            exit(-1);
        }
    }

    delete sockfd;
    return 0;
}