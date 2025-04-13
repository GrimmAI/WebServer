#include "HttpServer.h"
#include "../tcp/TcpServer.h"
#include <functional>
#include "HttpParase.h"
#include "../tcp/Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

HttpServer::HttpServer(std::string ip, int port) {
    tcp_server_ = std::make_unique<TcpServer>(ip, port);

    std::function<void(Connection*)> cb = std::bind(&HttpServer::HandleRequestMessage, this, std::placeholders::_1);
    tcp_server_->SetHandleMessageCallback(std::move(cb));

}

HttpServer::~HttpServer() {

}

void HttpServer::HandleRequestMessage(Connection *conn) {
    HttpParase *parase = conn->GetHttpParase();
    conn->Read();
    if (!parase->parase_message(conn->GetReadBuffer().c_str(), conn->GetReadBuffer().size())) {
        conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->Close();
    }

    if (parase->get_complete_request()) {
        SendResponseMessage(conn, *parase->get_http_request());
        parase->reset_context_status();
    }
}

void HttpServer::SendResponseMessage(Connection *conn, const HttpRequest &request) {
    std::string connection_state = request.GetHeader("Connection");
    bool close = (connection_state == "Close" || (request.version() == HttpRequest::Version::kHttp10 &&
                  connection_state != "keep-alive"));
    HttpResponse response(close);
    build_response_message_callback_(request, &response);

    conn->Send(response.message().c_str());

    if(response.IsCloseConnection()) {
        conn->Close();
    }
}

// 设置业务回调函数
void HttpServer::SetBuildResponseMessageCallback(std::function<void(const HttpRequest &, HttpResponse *)> &&func) {
    build_response_message_callback_ = std::move(func);
}

void HttpServer::Start() {
    tcp_server_->Start();
}
