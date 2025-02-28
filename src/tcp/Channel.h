#pragma once
#include <sys/epoll.h>

class Epoll;
class Channel {
private:
    int fd;
    Epoll* ep;
    uint32_t events;
    uint32_t pre_events;
    bool inEpoll;
public:
    Channel(Epoll* _ep, int _fd);
    ~Channel();

    void enableReading();

    int get_fd();
    uint32_t get_events();
    uint32_t get_pre_events();
    bool getInEpoll();
    void setInEpoll();
    void setRevents(uint32_t);
};