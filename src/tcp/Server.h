#pragma once
#include <map>

class EventLoop;
class Acceptor;
class Socket;
class Connection;
class Server {
private:
    EventLoop* lp;
    Acceptor* acceptor;
    std::map<int, Connection*> connections;
public:
    Server(EventLoop*);
    ~Server();

    void newConnection(Socket*);
    void deleteConnection(Socket*);
};