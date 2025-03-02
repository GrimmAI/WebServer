#pragma once
#include <memory>
#include <string>

class Connection;
class Server;
class HttpRequest;
class HttpResponse;
class HttpParase;
class HttpServer {
private:
    std::unique_ptr<Server> tcp_server;
    // std::unique_ptr<HttpRequest> request;
    // std::unique_ptr<HttpResponse> response;
    // std::unique_ptr<HttpParase> http_parase;
public:
    HttpServer(std::string, int);
    ~HttpServer();
    void handle_request_message(Connection*);
    void send_response_message(Connection*);
};