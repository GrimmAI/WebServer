#include "Epoll.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

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

void Epoll::addfd(int sockfd, int opcode) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof ev);
    ev.data.fd = sockfd;
    ev.events = opcode;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
}

std::vector<epoll_event> Epoll::poll() {
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    std::vector<epoll_event> activeEvents;
    for (int i = 0; i < nfds; i++)
        activeEvents.push_back(events[i]);
    return activeEvents;
}

