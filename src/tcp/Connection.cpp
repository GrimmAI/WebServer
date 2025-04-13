#include "Connection.h"
#include "Channel.h"
#include <cstring>
#include <unistd.h>
#include "Buffer.h"
#include <fcntl.h>
#include <iostream>
#include "../http/HttpParase.h"

Connection::Connection(std::shared_ptr<EventLoop> lp, int serv_sockfd, std::string ip, int port)
                       : lp_(lp), serv_sockfd_(serv_sockfd) {
    memset(&clnt_addr_, 0, sizeof clnt_addr_);
    clnt_addr_.sin_family = AF_INET;
    clnt_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    clnt_addr_.sin_port = htons(port);

    socklen_t clnt_addr_len = sizeof(clnt_addr_);

    clnt_sockfd_ = accept(serv_sockfd, (sockaddr*)&clnt_addr_, &clnt_addr_len);
    fcntl(clnt_sockfd_, F_SETFL, fcntl(clnt_sockfd_, F_GETFL) | O_NONBLOCK);

    clnt_Channel_ = std::make_unique<Channel>(lp, clnt_sockfd_);
    clnt_Channel_->EnableReading();
    std::function<void()> cb = std::bind(&Connection::HandleMessage, this);
    clnt_Channel_->SetEventCallback(std::move(cb));

    readBuffer_ = std::make_unique<Buffer>();
    sendBuffer_ = std::make_unique<Buffer>();

    http_parase_ = std::make_unique<HttpParase>();
    connection_state_ = ConnectionState::Connected;
}

HttpParase * Connection::GetHttpParase() {
    return http_parase_.get();
}

Connection::~Connection() {
    if (connection_state_ == ConnectionState::Connected) {
        delete_connection_callback_(clnt_sockfd_);
    }
}

void Connection::SetHandleMessageCallback(std::function<void(Connection *)> &&func) {
    handle_message_callback_ = std::move(func);
}

void Connection::SetDeleteConnectionCallback(std::function<void(int)> &&func) {
    delete_connection_callback_ = std::move(func);
}

void Connection::Close() {
    if (connection_state_ == ConnectionState::Connected) {
        delete_connection_callback_(clnt_sockfd_);
    }
}

void Connection::HandleMessage() {
    // std::cout << "handle_message callback\n";
    handle_message_callback_(this);
}

std::string Connection::GetReadBuffer() {
    return readBuffer_->get_buf();
}

void Connection::SetSendBuf(const char* str) {
    sendBuffer_->setBuf(str); 
}

void Connection::Send(const std::string &msg){
    SetSendBuf(msg.c_str());
    Write();
}

void Connection::Send(const char* msg){
    SetSendBuf(msg);
    Write();
}

void Connection::Read() {
    readBuffer_->clear();
    ReadNonBlocking();
}

void Connection::Write() {
    WriteNonBlocking();
    sendBuffer_->clear();
}

void Connection::ReadNonBlocking() {
    char buf[1024];
    while(true){
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_read = ::read(clnt_sockfd_, buf, sizeof(buf));
        if(bytes_read > 0){
            readBuffer_->append(buf, bytes_read);
        }else if(bytes_read == -1 && errno == EINTR){
            //std::cout << "continue reading" << std::endl;
            continue;
        }else if((bytes_read == -1) && (
            (errno == EAGAIN) || (errno == EWOULDBLOCK))){
            break;
        }else if (bytes_read == 0){//
            delete_connection_callback_(clnt_sockfd_);
            break;
        }else{
            delete_connection_callback_(clnt_sockfd_);
            break;
        }
    }
}

void Connection::WriteNonBlocking() {
    char buf[sendBuffer_->size()];
    memcpy(buf, sendBuffer_->c_str(), sendBuffer_->size());
    int data_size = sendBuffer_->size();
    int data_left = data_size;

    while (data_left > 0) {
        ssize_t bytes_write = ::write(clnt_sockfd_, buf + data_size - data_left, data_left);
        if(bytes_write == -1 && errno == EINTR){
            //std::cout << "continue writing" << std::endl;
            continue;
        }
        if(bytes_write == -1 && errno == EAGAIN){
            break;
        }
        if(bytes_write == -1){
            delete_connection_callback_(clnt_sockfd_);
            break;
        }
        data_left -= bytes_write;
    }
}