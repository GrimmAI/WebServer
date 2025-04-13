#include "Epoll.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include "Channel.h"
#include <iostream>

#define MAX_EVENTS 100

Epoll::Epoll() : epfd_(-1), events_(nullptr) {
    epfd_ = epoll_create1(0);
    events_ = new epoll_event[MAX_EVENTS];
}

Epoll::~Epoll() {
    if(epfd_ != -1) {
        close(epfd_);
        epfd_ = -1;
    }
    delete [] events_;
}

// 遍历就绪队列
std::vector<Channel *> Epoll::Poll() {
    int nfds = epoll_wait(epfd_, events_, MAX_EVENTS, -1);
    std::vector<Channel *> active_channel;
    for (int i = 0; i < nfds; i++) {
        Channel *ch = (Channel *)events_[i].data.ptr;
        ch->SetRevents(events_[i].events);
        active_channel.push_back(ch);
    }
    return active_channel;
}

void Epoll::UpdateChannel(Channel *ch) {
    int fd = ch->GetFd();
    struct epoll_event ev;
    memset(&ev, 0, sizeof ev);
    ev.data.ptr = ch;
    ev.events = ch->GetEvents();   // 拿到Channel希望监听的事件
    if(!ch->GetInEpoll()) {
        epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev); //添加Channel中的fd到epoll
        ch->SetInEpoll(true);
    } else{
        epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev); //已存在，则修改
    }
}

void Epoll::DeleteChannel(Channel* ch) const {
    if (!ch->GetInEpoll()) {
        return;
    }
    int sockfd = ch->GetFd();
    if (epoll_ctl(epfd_, EPOLL_CTL_DEL, sockfd, nullptr) == -1) {
        std::cout << "Epoller::UpdateChannel epoll_ctl_del failed" << std::endl;
    }

    ch->SetInEpoll(false);
}

