#include "HttpServer.h"
#include "../tcp/Server.h"
#include <functional>
#include "HttpParase.h"
#include "../tcp/Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

HttpServer::HttpServer(std::string ip, int port) {
    tcp_server = std::make_unique<Server>(ip, port);

    std::function<void(Connection*)> cb = std::bind(&HttpServer::handle_request_message, this, std::placeholders::_1);
    tcp_server->set_handle_message_callback(cb);

    
}

HttpServer::~HttpServer() {

}

void HttpServer::handle_request_message(Connection* conn) {
    HttpParase* parase = conn->get_http_parase();
    conn->read();
    if (!parase->parase_message(conn->get_read_buffer().c_str(), conn->get_read_buffer().size())) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        // conn->close();
    }

    if (parase->get_complete_request()) {
        send_response_message(conn, *parase->get_http_request());
        parase->reset_context_status();
    }
}

void HttpServer::send_response_message(Connection* conn, const HttpRequest& request) {
    std::string connection_state = request.GetHeader("Connection");
    bool close = (connection_state == "Close" || (request.version() == HttpRequest::Version::kHttp10 &&
                  connection_state != "keep-alive"));
    HttpResponse response(close);
    build_response_message_callback(request, &response);

    conn->send(response.message().c_str());

    if(response.IsCloseConnection()) {
        // conn->close();
    }
}

void HttpServer::set_build_message_callback(std::function<void(const HttpRequest&, HttpResponse*)> func) {
    build_response_message_callback = func;
}

void HttpServer::start() {
    tcp_server->start();
}
