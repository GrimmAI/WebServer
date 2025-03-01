#pragma once
#include <vector>
#include <memory>
class Epoll;
class Channel;
class EventLoop {
private:
    std::unique_ptr<Epoll> ep;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void update_channel(Channel*);
};