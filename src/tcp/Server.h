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
class Server {
private:
    std::unique_ptr<EventLoop> main_reactor;
    std::unique_ptr<Acceptor> acceptor;
    std::map<int, Connection*> connections;
    std::unique_ptr<ThreadPool> thread_pool;
    std::vector<std::unique_ptr<EventLoop>> sub_reactor;
    int idx;
    std::string ip;
    int port;
    std::function<void(Connection*)> handle_message_callback;
public:
    Server(std::string, int);
    ~Server();

    void newConnection(int);
    void deleteConnection(int);
    void set_handle_message_callback(std::function<void(Connection*)>);
    void start();
};