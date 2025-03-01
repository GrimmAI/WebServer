#pragma once
#include <functional>
#include <arpa/inet.h>
#include <string>
#include <memory>
class EventLoop;
class Buffer;
class Connection {
private:
    EventLoop* lp;
    int clnt_sockfd;
    struct sockaddr_in clnt_addr;
    int serv_sockfd;
    std::function<void(int)> delete_connection_callback;
    std::function<void(int)> handle_message_callback;
    std::unique_ptr<Buffer> readBuffer;
    std::unique_ptr<Buffer> sendBuffer;
public:
    Connection(EventLoop*, int, std::string, int);
    ~Connection();
    void set_delete_connection_callback(std::function<void(int)>);
    void set_handle_message_callback(std::function<void(int)>);
    void handle_message();

    void set_send_buf(const char*);
    void read();
    void send(const std::string &);
    void send(const char*);
    void write();
    void ReadNonBlocking();
    void WriteNonBlocking();
};