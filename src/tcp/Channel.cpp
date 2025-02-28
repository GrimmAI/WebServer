#include "Channel.h"
#include "Epoll.h"

Channel::Channel(Epoll* _ep, int _fd) : ep(_ep), fd(_fd), events(0), pre_events(0), inEpoll(false) {

}

Channel::~Channel() {

}

void Channel::enableReading() {
    events = EPOLLIN | EPOLLET;
    ep->update_channel(this);
}

int Channel::get_fd() {
    return fd;
}

uint32_t Channel::get_events() {
    return events;
}

uint32_t Channel::get_pre_events() {
    return pre_events;
}

bool Channel::getInEpoll() {
    return inEpoll;
}

void Channel::setInEpoll() {
    inEpoll = true;
}

void Channel::setRevents(uint32_t events) {
    pre_events = events;
}