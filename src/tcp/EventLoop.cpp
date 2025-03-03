#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include "../timer/TimeStamp.h"
#include "../timer/TimerQueue.h"

EventLoop:: EventLoop() {
    ep = std::make_unique<Epoll>();
    timer_queue_ = std::make_unique<TimerQueue>(this);
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

void EventLoop::delete_channel(Channel* ch) {
    ep->delete_channel(ch);
}

void EventLoop::RunAt(TimeStamp timestamp, std::function<void()>const & cb){
    timer_queue_->AddTimer(timestamp, std::move(cb), 0.0);
}

void EventLoop::RunAfter(double wait_time, std::function<void()> const & cb){
    TimeStamp timestamp(TimeStamp::AddTime(TimeStamp::Now(), wait_time));
    timer_queue_->AddTimer(timestamp, std::move(cb), 0.0);
}

void EventLoop::RunEvery(double interval, std::function<void()> const & cb){
    TimeStamp timestamp(TimeStamp::AddTime(TimeStamp::Now(), interval));
    timer_queue_->AddTimer(timestamp, std::move(cb), interval);
}
