#include "HttpParase.h"
#include "HttpRequest.h"
#include <memory>
#include <string>
#include <algorithm>
#include <iostream>

HttpParase::HttpParase() : state(HttpRequestParaseState::START) {
    http_request = std::make_unique<HttpRequest>();
}

HttpParase::~HttpParase(){

};

HttpRequest* HttpParase::get_http_request() {
    return http_request.get();
}

bool HttpParase::get_complete_request() {
    return state == HttpRequestParaseState::COMPLETE;
}

void HttpParase::reset_context_status() {
    state = HttpRequestParaseState::START;
}

bool HttpParase::parase_message(const char* begin, int size) {
    char* start = const_cast<char*>(begin);
    char* end = start;
    char* colon = end; // 对于URL:PARAMS 和 HeaderKey: HeaderValue保存中间的位置
    while(state != HttpRequestParaseState::kINVALID && state != HttpRequestParaseState::COMPLETE
        && end - begin <= size) {

        char ch = *end; // 当前字符
        //std::cout << ch;
        switch (state) {
            case HttpRequestParaseState::START:{ 
                if (ch == CR || ch== LF || isblank(ch)) {
                    // 遇到空格，换行和回车都继续。
                } else if(isupper(ch)) {
                    // 遇到大写字母，说明遇到了METHOD
                    state = HttpRequestParaseState::METHOD;
                } else{
                    state = HttpRequestParaseState::kINVALID;
                }
                break;
            }
            case HttpRequestParaseState::METHOD:{
                if (isupper(ch)) {
                    // 如果是大写字母，则继续
                } else if(isblank(ch)) {
                    // 遇到空格表明，METHOD方法解析结束，当前处于即将解析URL，start进入下一个位置
                    http_request->SetMethod(std::string(start, end));
                    state = HttpRequestParaseState::BEFORE_URL;
                    start = end + 1; // 更新下一个指标的位置
                }else{
                    state = HttpRequestParaseState::kINVALID;
                }
                break;
            }
            case HttpRequestParaseState::BEFORE_URL:{
                // 对请求连接前的处理，请求连接以'/'开头
                if(ch == '/') {
                    // 遇到/ 说明遇到了URL，开始解析
                    state = HttpRequestParaseState::IN_URL;
                } else if (isblank(ch)){

                } else{
                    state = HttpRequestParaseState::kINVALID;
                }
                break;
            }
            case HttpRequestParaseState::IN_URL:{
                // 进入url中s
                if(ch == '?'){
                    // 当遇到?时，表明进入了request params的处理。
                    http_request->SetUrl(std::string(start, end));
                    start = end + 1;
                    state = HttpRequestParaseState::BEFORE_URL_PARAM_KEY;
                   
                    
                }else if (isblank(ch)){
                    // 说明没有请求参数，请求路径完成
                    http_request->SetUrl(std::string(start, end));
                    start = end + 1;
                    state = HttpRequestParaseState::BEFORE_PROTOCOL;
                }
                break;
            }
            case HttpRequestParaseState::BEFORE_URL_PARAM_KEY:{
                //std::cout << ch << std::endl;
                if(isblank(ch) || ch == CR || ch == LF){
                    // 当开始进入url params时，遇到了空格，换行等，则不合法
                    // std::cout << ch << std::endl;
                    state = HttpRequestParaseState::kINVALID;
                }else{
                    state = HttpRequestParaseState::URL_PARAM_KEY;
                }
                break;
            }
            case HttpRequestParaseState::URL_PARAM_KEY:{
                if(ch == '='){
                    // 遇到= 说明key解析完成
                    colon = end;
                    state = HttpRequestParaseState::BEFORE_URL_PARAM_VALUE;
                }else if(isblank(ch)){
                    state = HttpRequestParaseState::kINVALID;
                }
                break;
            }
            case HttpRequestParaseState::BEFORE_URL_PARAM_VALUE:{
                if(isblank(ch) || ch == LF || ch == CR){
                    state = HttpRequestParaseState::kINVALID;
                }else{
                    state = HttpRequestParaseState::URL_PARAM_VALUE;
                }
                break;
            }

            case HttpRequestParaseState::URL_PARAM_VALUE:{
                if(ch == '&'){
                    // 说明遇到了下一个请求参数
                    state = HttpRequestParaseState::BEFORE_URL_PARAM_KEY;
                    http_request->SetRequestParams(std::string(start, colon), std::string(colon + 1, end));
                    start = end + 1;
                }
                if(isblank(ch)){
                    // 遇到空格，说明解析结束。
                    http_request->SetRequestParams(std::string(start, colon), std::string(colon + 1, end));
                    start = end + 1;
                    state = HttpRequestParaseState::BEFORE_PROTOCOL;
                }else{

                }
                break;
            }
            case HttpRequestParaseState::BEFORE_PROTOCOL:{
                //std::cout << std::string(start, end) << std::endl;
                if(isblank(ch)){
                    // nothing
                }else{
                    state = HttpRequestParaseState::PROTOCOL;
                }
                break;
            }
            case HttpRequestParaseState::PROTOCOL:{
                if(ch == '/'){
                    http_request->SetProtocol(std::string(start, end));
                    start = end + 1;
                    state = HttpRequestParaseState::BEFORE_VERSION;
                }else{

                }
                break;
            }

            case HttpRequestParaseState::BEFORE_VERSION:{
                if(isdigit(ch)){
                    state = HttpRequestParaseState::VERSION;
                }else{
                    state = HttpRequestParaseState::kINVALID;
                }
                break;
            }

            case HttpRequestParaseState::VERSION:{
                if(ch == CR){
                    // 说明结束了
                    http_request->SetVersion(std::string(start, end));
                    start = end + 1;
                    state = HttpRequestParaseState::WHEN_CR;
                }else if(isdigit(ch) || ch == '.'){
                    
                }else{
                    state = HttpRequestParaseState::kINVALID;
                }
                break;
            }

            // 需要注意的是，对header的解析并不鲁棒
            case HttpRequestParaseState::HEADER_KEY:{
                if(ch == ':'){
                    colon = end;
                    state = HttpRequestParaseState::HEADER_VALUE;
                }
                break;
            }
            case HttpRequestParaseState::HEADER_VALUE:{
                if(isblank(ch)){

                }else if(ch == CR){
                    http_request->AddHeader(std::string(start, colon), std::string(colon + 2, end));
                    start = end + 1;
                    state = HttpRequestParaseState::WHEN_CR;
                }
                break;
            }

            case HttpRequestParaseState::WHEN_CR:{
                if(ch == LF){
                    // 如果遇到了'\n'之后遇到了'\r'，那就意味着这一行结束了
                    start = end + 1;
                    state = HttpRequestParaseState::CR_LF;
                }else{
                    state = HttpRequestParaseState::kINVALID;
                }
                break;
            }
            case HttpRequestParaseState::CR_LF:{
                //std::cout << "111" << ch << std::endl;
                if(ch == CR){
                    // 说明遇到了空行，大概率时结束了
                    state = HttpRequestParaseState::CR_LF_CR;
                    //start  = end + 1;
                    //std::cout << "a:" << (*start == '\n') << std::endl;
                    //std::cout << "b:" << (*end == '\r') << std::endl;
                }else if(isblank(ch)){
                    state = HttpRequestParaseState::kINVALID;
                }else{
                    state = HttpRequestParaseState::HEADER_KEY;
                }
                break;
            }
            case HttpRequestParaseState::CR_LF_CR:{ 
                // 判断是否需要解析请求体
                //
                //std::cout << "c:" << (ch == '\n') << std::endl;
                //std::cout << "size:" << end-begin << std::endl;
                if(ch == LF){
                    // 这就意味着遇到了空行，要进行解析请求体了
                    if(http_request-> headers().count("Content-Lenght")){
                        if(atoi(http_request->GetHeader("Content-Lenght").c_str()) > 0){
                            state = HttpRequestParaseState::BODY;
                        }else{
                            state = HttpRequestParaseState::COMPLETE;
                        }
                    }else{
                        if(end - begin < size){
                            state = HttpRequestParaseState::BODY;
                        }else{
                            state = HttpRequestParaseState::COMPLETE;
                        }
                    }
                    start = end + 1;
                }else{
                    state = HttpRequestParaseState::kINVALID;
                }
                break;
            }

            case HttpRequestParaseState::BODY:{
                
                int bodylength = size - (end - begin);
                //std::cout << "bodylength:" << bodylength << std::endl;
                http_request->SetBody(std::string(start, start + bodylength));
                state = HttpRequestParaseState::COMPLETE;
                break;
            }

            default:
                state = HttpRequestParaseState::kINVALID;
                break;
        } // switch
        end++;
    } // while
    
    return state == HttpRequestParaseState::COMPLETE;
}

