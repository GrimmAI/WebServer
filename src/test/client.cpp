#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "tcp/Buffer.h"
#include <sys/socket.h>
#include <arpa/inet.h>



int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8888);
    // sock->setnonblocking(); 客户端使用阻塞式连接比较好，方便简单不容易出错
    connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr));

    Buffer* sendBuffer = new Buffer();
    Buffer* readBuffer = new Buffer();
    
    while(true) {
        sendBuffer->getline();
        ssize_t write_bytes = write(sockfd, sendBuffer->c_str(), sendBuffer->size());
        if(write_bytes == -1){
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        int already_read = 0;
        char buf[1024];    //这个buf大小无所谓
        while(true){
            bzero(&buf, sizeof(buf));
            ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
            if(read_bytes > 0){
                readBuffer->append(buf, read_bytes);
                already_read += read_bytes;
            } else if(read_bytes == 0){         //EOF
                printf("server disconnected!\n");
                exit(EXIT_SUCCESS);
            }
            if(already_read >= sendBuffer->size()){
                printf("message from server: %s\n", readBuffer->c_str());
                break;
            } 
        }
        readBuffer->clear();
    }

    delete sendBuffer;
    delete readBuffer;
    return 0;
}