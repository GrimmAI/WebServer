#pragma once
#include <functional>
#include <arpa/inet.h>
#include <string>
#include <memory>

class Channel;
class EventLoop;
class Buffer;
class HttpParase;
// 管理客户端与当前服务端的一个连接
class Connection {
    friend class TcpServer;
private:
    std::shared_ptr<EventLoop> lp_;
    int clnt_sockfd_;
    struct sockaddr_in clnt_addr_;
    int serv_sockfd_;
    std::function<void(int)> delete_connection_callback_;
    std::function<void(Connection *)> handle_message_callback_;
    std::unique_ptr<Buffer> readBuffer_;
    std::unique_ptr<Buffer> sendBuffer_;
    std::unique_ptr<Channel> clnt_Channel_;
    std::unique_ptr<HttpParase> http_parase_;

public:
    enum ConnectionState {
        Invalid = -1,
        Connected,
        Disconected
    };
    ConnectionState connection_state_;
    Connection(std::shared_ptr<EventLoop>, int, std::string, int);
    ~Connection();
    void SetDeleteConnectionCallback(std::function<void(int)> &&);
    void SetHandleMessageCallback(std::function<void(Connection *)> &&);
    void HandleMessage();

    std::string GetReadBuffer();
    void Close();
    void SetSendBuf(const char*);
    void Read();
    void Send(const std::string &);
    void Send(const char*);
    void Write();
    void ReadNonBlocking();
    void WriteNonBlocking();
    HttpParase* GetHttpParase();
};