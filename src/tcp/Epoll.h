#pragma once
#include <sys/epoll.h>
#include <vector>

class Epoll {
private:
    int epfd;
    struct epoll_event* events;
public:
    Epoll(int);
    Epoll();
    ~Epoll();
    
};