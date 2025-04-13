#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"

EventLoop:: EventLoop() {
    ep_ = std::make_unique<Epoll>();
}

EventLoop::~EventLoop() {

}

void EventLoop::Loop() {
    while (true) {
        std::vector<Channel *> active_channel = ep_->Poll();
        for (auto &ch: active_channel)
            ch->HandleEvent();
    }
}

void EventLoop::UpdateChannel(Channel *ch) {
    ep_->UpdateChannel(ch);
}

void EventLoop::DeleteChannel(Channel *ch) {
    ep_->DeleteChannel(ch);
}
