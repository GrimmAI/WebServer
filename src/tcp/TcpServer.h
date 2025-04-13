#pragma once
#include <map>
#include <vector>
#include <string>
#include <functional>
#include <memory>

class EventLoop;
class Acceptor;
class Connection;
class ThreadPool;
class EventLoop;
class TcpServer {
private:
    std::shared_ptr<EventLoop> main_reactor_;
    std::unique_ptr<Acceptor> acceptor_;
    std::map<int, std::shared_ptr<Connection>> connections_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::vector<std::shared_ptr<EventLoop>> sub_reactor_;
    int sub_reactor_idx_;
    std::string server_ip_;
    int server_port_;
    std::function<void(Connection *)> handle_message_callback_;
public:
    TcpServer(std::string, int);
    ~TcpServer();

    void NewConnection(int);
    void DeleteConnection(int);
    void SetHandleMessageCallback(std::function<void(Connection *)> &&);
    void Start();
};