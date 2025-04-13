#pragma once
#include <sys/epoll.h>
#include <vector>

// Epoll I/O复用机制，Linux 内核中的一组数据结构 + 系统调用
// Epoll 是内核通过红黑树、就绪链表和回调机制实现的高效 I/O 事件管理工具。
class Channel;
class Epoll {
private:
    int epfd_;
    struct epoll_event *events_;
public:
    Epoll();
    ~Epoll();
    std::vector<Channel *> Poll();
    void UpdateChannel(Channel *);
    void DeleteChannel(Channel *) const;
};