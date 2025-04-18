#include <iostream>
#include <unistd.h>
#include <string.h>
#include <functional>
#include "tcp/Buffer.h"
#include "tcp/ThreadPool.h"
#include <arpa/inet.h>
#include <sys/socket.h>


using namespace std;

void oneClient(int msgs, int wait){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8888);
    // sock->setnonblocking(); 客户端使用阻塞式连接比较好，方便简单不容易出错
    connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr));



    Buffer *sendBuffer = new Buffer();
    Buffer *readBuffer = new Buffer();

    sleep(wait);
    int count = 0;
    while(count < msgs){
        sendBuffer->setBuf("I'm client!");
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
                printf("count: %d, message from server: %s\n", count++, readBuffer->c_str());
                break;
            } 
        }
        readBuffer->clear();
    }
}

int main(int argc, char *argv[]) {
    int threads = 100;
    int msgs = 100;
    int wait = 0;
    int o;
    const char *optstring = "t:m:w:";
    while ((o = getopt(argc, argv, optstring)) != -1) {
        switch (o) {
            case 't':
                threads = stoi(optarg);
                break;
            case 'm':
                msgs = stoi(optarg);
                break;
            case 'w':
                wait = stoi(optarg);
                break;
            case '?':
                printf("error optopt: %c\n", optopt);
                printf("error opterr: %d\n", opterr);
                break;
        }
    }

    ThreadPool *poll = new ThreadPool(threads);
    std::function<void()> func = std::bind(oneClient, msgs, wait);
    for(int i = 0; i < threads; ++i){
        poll->enqueue(func);
    }
    delete poll;
    return 0;
}
