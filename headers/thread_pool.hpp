#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

class ThreadPool
{
public:
    ThreadPool(size_t numThreads) : stop(false)
    {
        for (size_t i = 0; i < numThreads; ++i)
            workers.emplace_back([this]
                                 {
                                     while (!stop)
                                     {
                                         std::packaged_task<void()> task;
                                         {
                                             std::unique_lock<std::mutex> lock(queueMutex);
                                             condition.wait(lock, [this]
                                                            { return stop || !tasks.empty(); });
                                             if (stop && tasks.empty())
                                                 return;
                                             task = std::move(tasks.front());
                                             tasks.pop();
                                         }
                                         task();
                                     } });
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

    template <typename Func>
    void enqueue(Func f)
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(std::packaged_task<void()>(f));
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::packaged_task<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};
