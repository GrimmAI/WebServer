#include <iostream>
#include "../http/HttpServer.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../tcp/EventLoop.h"
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <json/json.h> // 使用 JSON for Modern C++ 库来处理 JSON 数据

struct ImageResult {
    std::string description;
    float score;
    std::string local_path;
};

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

std::string generate_html(const std::vector<ImageResult>& images) {
    // 构建JSON数组
    Json::Value jsonData;
    for (const auto& img : images) {
        Json::Value item;
        item["metadata"] = img.description;  // 需要UTF-8编码
        item["similarity"] = img.score;
        item["url"] = img.local_path;        // 原始本地路径
        jsonData.append(item);
    }

    // 安全转义
    std::string jsonStr = Json::FastWriter().write(jsonData);
    std::string safeJson;
    for (char c : jsonStr) {
        if (c == '<') safeJson += "\\u003c";
        else if (c == '>') safeJson += "\\u003e";
        else if (c == '&') safeJson += "\\u0026";
        else safeJson += c;
    }

    // 插入模板
    std::string html = load_html();
    size_t pos = html.find("<script id=\"__INITIAL_JSON__\"");
    if (pos != std::string::npos) {
        html.replace(pos, 31, "<script id=\"__INITIAL_JSON__\" type=\"application/json\">" + safeJson + "</script>");
    }
    return html;
}


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
            std::string query = requestJson["query"].asString();

            // // 模拟图片检索逻辑
            // std::vector<std::string> image_urls = {
            //     "/home/mazhaomeng/cpp/WebServer/src/test/images/test.png"
            // };

            // // 构造 JSON 格式的响应
            // Json::Value responseJson;
            // for (const auto& url : image_urls) {
            //     Json::Value image;
            //     image["url"] = url;
            //     image["similarity"] = 0.92; // 模拟相似度
            //     image["metadata"] = "金毛犬在公园草地奔跑"; // 模拟元数据
            //     responseJson.append(image);
            // }

            // // 将 JSON 响应转换为字符串
            // Json::StreamWriterBuilder writerBuilder;
            // std::string jsonResponse = Json::writeString(writerBuilder, responseJson);


            // // 读取HTML模板文件
            // std::string html_template = load_html();

            // // 在HTML模板中预留一个位置用于插入JSON数据
            // std::string html_content = html_template;
            // std::cout << jsonResponse << std::endl;
            // size_t pos = html_content.find("<!-- JSON_DATA_PLACEHOLDER -->");
            // if (pos != std::string::npos) {
            //     html_content.replace(pos, strlen("<!-- JSON_DATA_PLACEHOLDER -->"), "<script>var jsonResponse = " + jsonResponse + ";</script>");
            //     std::cout << html_content << std::endl;
            // }

            auto t = new ImageResult();
            t->local_path = "https://picsum.photos/300/200?random=2";
            std::vector<ImageResult> a;
            a.push_back(*t);
            auto html_content = generate_html(a);

            // 设置响应
            response->SetStatusCode(HttpResponse::HttpStatusCode::k200K);
            response->SetBody(html_content);
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
    return 0;
}