#pragma once
#include <sys/epoll.h>
#include <functional>

class EventLoop;
class Channel {
private:
    int fd;
    EventLoop* lp;
    uint32_t events;
    uint32_t pre_events;
    bool inEpoll;
    std::function<void()> cb;

public:
    Channel(EventLoop*, int);
    ~Channel();

    void enableReading();

    int get_fd();
    uint32_t get_events();
    uint32_t get_pre_events();
    bool getInEpoll();
    void setInEpoll();
    void setRevents(uint32_t);
    void handleEvent();
    void set_event_callback(std::function<void()>);
};