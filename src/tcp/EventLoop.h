#pragma once
#include <vector>

class Epoll;
class Channel;
class EventLoop {
private:
    Epoll* ep;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void update_channel(Channel*);
};