#include "tcp/Socket.h"
#include "tcp/InetAddress.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "tcp/Buffer.h"

int main() {
    Socket* sockfd = new Socket();
    InetAddress* serv_addr = new InetAddress("127.0.0.1", 8888);

    sockfd->connect(serv_addr);

    Buffer* sendBuffer = new Buffer();
    Buffer* readBuffer = new Buffer();
    
    while(true){
        sendBuffer->getline();
        ssize_t write_bytes = write(sockfd->get_fd(), sendBuffer->c_str(), sendBuffer->size());
        if(write_bytes == -1){
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        int already_read = 0;
        char buf[1024];    //这个buf大小无所谓
        while(true){
            bzero(&buf, sizeof(buf));
            ssize_t read_bytes = read(sockfd->get_fd(), buf, sizeof(buf));
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
    delete sockfd;
    return 0;
}