#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "tcp/Epoll.h"
#include "tcp/Channel.h"
#include "tcp/EventLoop.h"
#include "tcp/Server.h"
#include "tcp/Buffer.h"

class EchoServer {
private:
    EventLoop* loop;
    Server* server;
    Buffer* readBuffer;

public:
    EchoServer(EventLoop* _loop, Server* _server) : loop(_loop), server(_server) {
        readBuffer = new Buffer();
        std::function<void(int)> cb = std::bind(&EchoServer::echo, this, std::placeholders::_1);
        server->set_handle_message_callback(cb);
    }

    ~EchoServer() {
        delete loop;
        delete server;
        delete readBuffer;
    }


    void start() {
        loop->loop();
    }

    void echo(int clnt_sockfd) {
        char buf[1024];
        while (true) {
            memset(buf, 0, sizeof buf);
            ssize_t read_bytes = read(clnt_sockfd, buf, sizeof buf);
            if (read_bytes > 0) {
                readBuffer->append(buf, read_bytes);
            } else if (read_bytes == -1 && errno == EINTR) {
                printf("continue reading");
                continue;
            } else if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                printf("finish reading once\n");
                printf("message from client fd %d: %s\n", clnt_sockfd, readBuffer->c_str());
                write(clnt_sockfd, readBuffer->c_str(), readBuffer->size());
                readBuffer->clear();
                break;
            } else if (read_bytes == 0) {
                printf("EOF, client fd %d disconnected\n", clnt_sockfd);
                close(clnt_sockfd);   //关闭socket会自动将文件描述符从epoll树上移除
                // delete_connection_callback(clnt_sockfd);
                break;
            }
        }
    }


};


int main(int argc, char *argv[]) {
    
    Server* server = new Server("127.0.0.1", 1234);

    server->set_handle_message_callback([](Connection *conn)
                                 {
        std::cout << "Message from client " << conn->id() << " is " << conn->read_buf()->c_str() << std::endl;
        conn->Send(conn->read_buf()->c_str()); });

    server->start();

    delete server;
    return 0;
}