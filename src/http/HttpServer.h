#pragma once
#include <memory>
#include <string>
#include <functional>

class Connection;
class Server;
class HttpRequest;
class HttpResponse;
class HttpServer {
private:
    std::unique_ptr<Server> tcp_server;
    std::function<void(const HttpRequest&, HttpResponse*)> build_response_message_callback;
public:
    HttpServer(std::string, int);
    ~HttpServer();
    void handle_request_message(Connection*);
    void send_response_message(Connection*, const HttpRequest&);

    void start();
    void set_build_message_callback(std::function<void(const HttpRequest&, HttpResponse*)>);
};