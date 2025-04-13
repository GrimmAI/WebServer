#include <iostream>
#include "../http/HttpServer.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../tcp/EventLoop.h"
#include <string>
#include <thread>
#include <fstream>
#include <sstream>

std::string load_html() {
    std::ifstream file("/home/mazhaomeng/cpp/WebServer/src/test/client_api.html");
    if (!file) {
        std::cerr << "ERROR: can not open HTML" << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

const std::string html = load_html();

void HttpResponseCallback(const HttpRequest &request, HttpResponse *response)
{
    if(request.method() != HttpRequest::Method::kGet){
        response->SetStatusCode(HttpResponse::HttpStatusCode::k400BadRequest);
        response->SetStatusMessage("Bad Request");
        response->SetCloseConnection(true);
    }

    {
        std::string url = request.url();
        if(url == "/"){
            response->SetStatusCode(HttpResponse::HttpStatusCode::k200K);
            response->SetBody(html);
            response->SetContentType("text/html");
        }else if(url == "/hello"){
            response->SetStatusCode(HttpResponse::HttpStatusCode::k200K);
            response->SetBody("hello world\n");
            response->SetContentType("text/plain");
        }else if(url == "/favicon.ico"){
            response->SetStatusCode(HttpResponse::HttpStatusCode::k200K);
        }else{
            response->SetStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
            response->SetStatusMessage("Not Found");
            response->SetBody("Sorry Not Found\n");
            response->SetCloseConnection(true);
        }
    }
    return;
}

int main(int argc, char *argv[]){
    std::cout << html << std::endl;
    int port;
    if (argc <= 1)
    {
        port = 1234;
    }else if (argc == 2){
        port = atoi(argv[1]);
    }else{
        printf("error");
        exit(0);
    }
    std::string ip = "127.0.0.1";
    HttpServer *server = new HttpServer(ip, port);
    server->SetBuildResponseMessageCallback(HttpResponseCallback);
    server->Start();

    delete server;
    return 0;
}