#pragma once
#include <sys/epoll.h>
#include <functional>
#include <memory>

// Channel是对fd的封装，
// 一个Channel类自始至终只负责一个文件描述符，
// 管理该描述符上的事件监听与回调。
class EventLoop;
class Channel {
private:
    std::shared_ptr<EventLoop> lp_;
    int fd_;
    uint32_t events_;
    uint32_t pre_events_;
    bool inEpoll_;
    std::function<void()> cb_;

public:
    Channel(std::shared_ptr<EventLoop>, int);
    ~Channel();

    void EnableReading();

    int GetFd();
    uint32_t GetEvents();
    uint32_t GetPreEvents();
    bool GetInEpoll();
    void SetInEpoll(bool);
    void SetRevents(uint32_t);
    void HandleEvent();
    void SetEventCallback(std::function<void()> &&);
};