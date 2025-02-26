#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

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
    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        printf("connect error");
        exit(-1);
    }

    while (true) {
        char buf[1024];
        memset(buf, 0, sizeof buf);
        scanf("%s", buf);
        ssize_t write_bytes = write(sockfd, buf, sizeof buf);
        if (write_bytes == -1) {
            printf("socket already disconnected, can't write any more\n");
            break;
        }
        memset(buf, 0, sizeof buf);
        ssize_t read_bytes = read(sockfd, buf, sizeof buf);
        if (read_bytes > 0) {
            printf("message from server: %s\n", buf);
        } else if (read_bytes == 0) {
            printf("server socket disconnected!\n");
            break;
        } else if (read_bytes == -1) {
            close(sockfd);
            printf("socket read error");
            exit(-1);
        }
    }

    close(sockfd);
    return 0;
}