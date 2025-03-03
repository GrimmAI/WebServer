#pragma once
#include <vector>
#include <memory>
class Epoll;
class Channel;
class TimerQueue;
class TimeStamp;
class EventLoop {
private:
    std::unique_ptr<Epoll> ep;
    std::unique_ptr<TimerQueue> timer_queue_;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void update_channel(Channel*);
    void delete_channel(Channel*);

    // 定时器功能，
    void RunAt(TimeStamp timestamp, std::function<void()> const & cb);
    void RunAfter(double wait_time, std::function < void()>const & cb);
    void RunEvery(double interval, std::function<void()> const & cb);
};