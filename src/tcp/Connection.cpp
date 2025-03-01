#include "Connection.h"
#include "Channel.h"
#include <cstring>
#include <unistd.h>
#include "Buffer.h"
#include <fcntl.h>


Connection::Connection(EventLoop* _lp, int _serv_sockfd, std::string ip, int port) : lp(_lp), serv_sockfd(_serv_sockfd) {
    memset(&clnt_addr, 0, sizeof clnt_addr);
    clnt_addr.sin_family = AF_INET;
    clnt_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    clnt_addr.sin_port = htons(port);

    socklen_t clnt_addr_len = sizeof(clnt_addr);

    clnt_sockfd = accept(serv_sockfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
    fcntl(clnt_sockfd, F_SETFL, fcntl(clnt_sockfd, F_GETFL) | O_NONBLOCK);

    std::unique_ptr clnt_Channel = std::make_unique<Channel>(lp, clnt_sockfd);
    clnt_Channel->enableReading();
    std::function<void()> callback = std::bind(&Connection::handle_message, this);
    clnt_Channel->set_event_callback(callback);

    readBuffer = std::make_unique<Buffer>();
    sendBuffer = std::make_unique<Buffer>();
}

Connection::~Connection() {
    delete_connection_callback(clnt_sockfd);
}

void Connection::set_handle_message_callback(std::function<void(int)> func) {
    handle_message_callback = func;
}

void Connection::set_delete_connection_callback(std::function<void(int)> func) {
    delete_connection_callback = func;
}

void Connection::handle_message() {
    handle_message_callback(clnt_sockfd);
}





void Connection::set_send_buf(const char* str) {
    sendBuffer->setBuf(str); 
}

void Connection::send(const std::string &msg){
    set_send_buf(msg.c_str());
    write();
}

void Connection::send(const char* msg){
    set_send_buf(msg);
    write();
}

void Connection::read() {
    readBuffer->clear();
    ReadNonBlocking();
}

void Connection::write() {
    WriteNonBlocking();
    sendBuffer->clear();
}


void Connection::ReadNonBlocking() {
    char buf[1024];
    while(true){
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_read = ::read(clnt_sockfd, buf, sizeof(buf));
        if(bytes_read > 0){
            readBuffer->append(buf, bytes_read);
        }else if(bytes_read == -1 && errno == EINTR){
            //std::cout << "continue reading" << std::endl;
            continue;
        }else if((bytes_read == -1) && (
            (errno == EAGAIN) || (errno == EWOULDBLOCK))){
            break;
        }else if (bytes_read == 0){//
            delete_connection_callback(clnt_sockfd);
            break;
        }else{
            delete_connection_callback(clnt_sockfd);
            break;
        }
    }
}

void Connection::WriteNonBlocking() {
    char buf[sendBuffer->size()];
    memcpy(buf, sendBuffer->c_str(), sendBuffer->size());
    int data_size = sendBuffer->size();
    int data_left = data_size;

    while (data_left > 0) {
        ssize_t bytes_write = ::write(clnt_sockfd, buf + data_size - data_left, data_left);
        if(bytes_write == -1 && errno == EINTR){
            //std::cout << "continue writing" << std::endl;
            continue;
        }
        if(bytes_write == -1 && errno == EAGAIN){
            break;
        }
        if(bytes_write == -1){
            delete_connection_callback(clnt_sockfd);
            break;
        }
        data_left -= bytes_write;
    }
}