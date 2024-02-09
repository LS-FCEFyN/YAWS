#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

/**
 * @class ThreadPool
 * @brief A thread pool for managing multiple threads of execution.
 *
 * This class provides a mechanism for spawning a fixed number of worker threads and
 * distributing tasks among them. It is designed to simplify the management of
 * concurrent operations within a multithreaded application.
 */
class ThreadPool
{
public:
    /**
     * @brief Constructs a ThreadPool with a specified number of threads.
     * @param numThreads The number of threads to be managed by the pool.
     */
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

    /**
     * @brief Destructor for the ThreadPool.
     *
     * Ensures that all threads finish executing their current tasks before
     * shutting down the pool.
     */
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

    /**
     * @brief Enqueues a task to be executed by the ThreadPool.
     * @tparam Func Callable type that represents the task.
     * @param f Function object representing the task to be executed.
     */
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
    /// Vector of worker threads.
    std::vector<std::thread> workers;
    /// Queue of tasks waiting to be executed.
    std::queue<std::packaged_task<void()>> tasks;
    /// Mutex for synchronizing access to the task queue.
    std::mutex queueMutex;
    /// Condition variable for signaling task availability.
    std::condition_variable condition;
    /// Flag indicating whether the ThreadPool should stop processing tasks.
    bool stop;
};