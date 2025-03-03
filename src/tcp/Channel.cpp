#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* _lp, int _fd) : lp(_lp), fd(_fd), events(0), pre_events(0), inEpoll(false), cb(nullptr) {

}

Channel::~Channel() {

}

void Channel::enableReading() {
    events = EPOLLIN | EPOLLET;
    lp->update_channel(this);
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

void Channel::setInEpoll(bool ok) {
    inEpoll = ok;
}

void Channel::setRevents(uint32_t events) {
    pre_events = events;
}

void Channel::handleEvent() {
    cb();
}

void Channel::set_event_callback(std::function<void()> func) {
    cb = func;
}

