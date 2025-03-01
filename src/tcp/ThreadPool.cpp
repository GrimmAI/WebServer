#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads = 10) {
    auto worker = [this]() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(this->queueMutex);
                this->condition.wait(lock, [this] {
                    return this->stop || !this->tasks.empty();
                });

                if (this->stop && this->tasks.empty()) {
                    return;  // 线程池停止且任务队列为空，退出线程
                }

                task = std::move(this->tasks.front());
                this->tasks.pop();
            }

            task();  // 执行任务
        }
    };

    for (size_t i = 0; i < numThreads; i++)
        workers.emplace_back(worker);
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;  // 设置停止标志
    }
    condition.notify_all();  // 唤醒所有线程,  notify_one()唤醒某一个线程，由调度器决定
    for (std::thread& worker : workers) {
        worker.join();  // 等待所有线程完成
    }
}