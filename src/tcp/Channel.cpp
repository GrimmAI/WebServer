#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"

Channel::Channel(std::shared_ptr<EventLoop> lp, int fd) : lp_(lp), fd_(fd), events_(0), 
                                                        pre_events_(0), inEpoll_(false), cb_(nullptr) {

}

Channel::~Channel() {

}

void Channel::EnableReading() {
    events_ = EPOLLIN | EPOLLET;
    lp_->UpdateChannel(this);
}

int Channel::GetFd() {
    return fd_;
}

uint32_t Channel::GetEvents() {
    return events_;
}

uint32_t Channel::GetPreEvents() {
    return pre_events_;
}

bool Channel::GetInEpoll() {
    return inEpoll_;
}

void Channel::SetInEpoll(bool in_epoll) {
    inEpoll_ = in_epoll;
}

void Channel::SetRevents(uint32_t events) {
    pre_events_ = events;
}

void Channel::HandleEvent() {
    cb_();
}

void Channel::SetEventCallback(std::function<void()> &&func) {
    cb_ = std::move(func);
}

