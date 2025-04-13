#pragma once
#include <memory>
#include <string>
#include <functional>

class Connection;
class TcpServer;
class HttpRequest;
class HttpResponse;
class HttpServer {
private:
    std::unique_ptr<TcpServer> tcp_server_;
    std::function<void(const HttpRequest &, HttpResponse *)> build_response_message_callback_;
public:
    HttpServer(std::string, int);
    ~HttpServer();
    void HandleRequestMessage(Connection*);
    void SendResponseMessage(Connection*, const HttpRequest&);

    void Start();
    void SetBuildResponseMessageCallback(std::function<void(const HttpRequest &, HttpResponse *)> &&);
};