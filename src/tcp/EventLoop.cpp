#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"

EventLoop:: EventLoop() {
    ep = std::make_unique<Epoll>();
}

EventLoop::~EventLoop() {

}

void EventLoop::loop() {
    while (true) {
        std::vector<Channel*> active_channel = ep->poll();
        for (auto &ch: active_channel)
            ch->handleEvent();
    }
}

void EventLoop::update_channel(Channel* ch) {
    ep->update_channel(ch);
}


