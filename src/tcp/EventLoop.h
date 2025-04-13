#pragma once
#include <vector>
#include <memory>
#include <functional>

// 通过一个无限循环不断监听事件、分发事件并执行对应的处理逻辑
class Epoll;
class Channel;
class EventLoop {
private:
    std::unique_ptr<Epoll> ep_;

public:
    EventLoop();
    ~EventLoop();

    void Loop();
    void UpdateChannel(Channel *);
    void DeleteChannel(Channel *);
};