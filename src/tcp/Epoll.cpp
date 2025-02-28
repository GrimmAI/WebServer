#include "Epoll.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include "Channel.h"

#define MAX_EVENTS 100

Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    events = new epoll_event[MAX_EVENTS];
}

Epoll::~Epoll() {
    if(epfd != -1){
        close(epfd);
        epfd = -1;
    }
    delete [] events;
}

std::vector<Channel*> Epoll::poll() {
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    std::vector<Channel*> active_channel;
    for (int i = 0; i < nfds; i++) {
        Channel *ch = (Channel*)events[i].data.ptr;
        ch->setRevents(events[i].events);
        active_channel.push_back(ch);
    }
    return active_channel;
}

void Epoll::update_channel(Channel* ch) {
    int fd = ch->get_fd();
    struct epoll_event ev;
    memset(&ev, 0, sizeof ev);
    ev.data.ptr = ch;
    ev.events = ch->get_events();   //拿到Channel希望监听的事件
    if(!ch->getInEpoll()) {
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);//添加Channel中的fd到epoll
        ch->setInEpoll();
    } else{
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);//已存在，则修改
    }
}

