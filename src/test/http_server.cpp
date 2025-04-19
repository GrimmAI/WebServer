#include <iostream>
#include "../http/HttpServer.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../tcp/EventLoop.h"
#include "../retrieval/Search.h"
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <json/json.h> // 使用 JSON for Modern C++ 库来处理 JSON 数据
// #include <opencv2/opencv.hpp>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <filesystem>


std::string get_filename(const std::string& path) {
    std::filesystem::path p(path);
    return p.filename().string(); // 直接获取文件名
}

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
Search *search = new Search();

void HttpResponseCallback(const HttpRequest &request, HttpResponse *response)
{
    std::string url = request.url();
    if(request.method() != HttpRequest::Method::kGet && request.method() != HttpRequest::Method::kPost){
        response->SetStatusCode(HttpResponse::HttpStatusCode::k400BadRequest);
        response->SetStatusMessage("Bad Request");
        response->SetCloseConnection(true);
    }


    if (request.method() == HttpRequest::Method::kGet) {
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
    } else if (request.method() == HttpRequest::Method::kPost) {
        if (url == "/api/search") {
            // 解析请求体中的 JSON 数据
            std::string body = request.body();
            Json::Value requestJson;
            Json::CharReaderBuilder readerBuilder;
            std::istringstream s(body);
            std::string errs;
            if (!Json::parseFromStream(readerBuilder, s, &requestJson, &errs)) {
                response->SetStatusCode(HttpResponse::HttpStatusCode::k400BadRequest);
                response->SetBody("Invalid JSON format\n");
                return;
            }

            // 获取用户输入的查询
            std::string query = requestJson["query"].asString() + "\n";
            // std::cout << "query: " << query << std::endl;
            auto query_embedding = search->GetUserQueryTextEmbedding(query);

            auto local_images = search->GetTopKResults(query_embedding, 12);

            // 转换本地路径为Web URL
            Json::Value json_data;
            for (const auto& local_path : local_images) {
                Json::Value item;
                item["url"] = "http://127.0.0.1:80/image/" + get_filename(local_path);
                item["similarity"] = 0.92;
                item["metadata"] = "金毛犬在公园草地奔跑";
                json_data.append(item);
            }

            // 序列化 JSON 数据
            Json::StreamWriterBuilder writer;
            std::string json_str = Json::writeString(writer, json_data);

            // 设置响应
            response->SetStatusCode(HttpResponse::HttpStatusCode::k200K);
            response->SetBody(json_str);
            response->SetContentType("application/json");
        } else {
            response->SetStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
            response->SetStatusMessage("Not Found");
            response->SetBody("Sorry Not Found\n");
            response->SetCloseConnection(true);
        }
    }
    return;
}


int main(int argc, char *argv[]){
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
    delete search;
    return 0;
}