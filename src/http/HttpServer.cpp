#include "HttpServer.h"
#include "tcp/Server.h"
#include <functional>
#include "HttpParase.h"
#include "tcp/Connection.h"

HttpServer::HttpServer(std::string ip, int port) {
    tcp_server = std::make_unique<Server>(ip, port);
    // request = std::make_unique<HttpRequest>();
    // response = std::make_unique<HttpResponse>();
    // http_parase = std::make_unique<HttpParase>();

    std::function<void(Connection*)> cb = std::bind(&HttpServer::handle_request_message, this, std::placeholders::_1);
    tcp_server->set_handle_message_callback(cb);
}

HttpServer::~HttpServer() {

}

void HttpServer::handle_request_message(Connection* conn) {
    HttpParase* parase = conn->get_http_parase();
    conn->read();
    if (!parase->ParaseRequest(conn->get_read_buffer().c_str(), conn->get_read_buffer().size())) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        // conn->close();
    }

    if (parase->GetCompleteRequest()) {
        onRequest(conn, *parase->request());
        parase->ResetContextStatus();
    }
}

void HttpServer::send_response_message(Connection* conn, const HttpRequest &request) {
    // std::string connection_state = request.GetHeader("Connection");
    // bool close = (connection_state == "Close" ||
    //               (request.version() == HttpRequest::Version::kHttp10 &&
    //               connection_state != "keep-alive"));
    HttpResponse response(close);
    response_callback_(request, &response);

    conn->send(response.message().c_str());

    if(response.IsCloseConnection()) {
        conn->HandleClose();
    }
}
