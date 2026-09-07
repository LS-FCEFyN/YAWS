#pragma once

// Standard library headers
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * @class ThreadPool
 * @brief A thread pool for managing multiple threads of execution.
 *
 * This class provides a mechanism for spawning a fixed number of worker threads and
 * distributing tasks among them. It is designed to simplify the management of
 * concurrent operations within a multithreading application.
 */
class ThreadPool
{
public:
    /**
     * @brief Constructs a ThreadPool with a specified number of threads.
     * @param numThreads The number of threads to be managed by the pool.
     */
    ThreadPool(size_t num_threads) : stop(false)
    {
        for (size_t i = 0; i < num_threads; ++i)
            workers.emplace_back([this]
                                 {
                                     while (!stop)
                                     {
                                         std::packaged_task<void()> task;
                                         {
                                             std::unique_lock<std::mutex> lock(queue_mutex);
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
            std::unique_lock<std::mutex> lock(queue_mutex);
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
    void enqueue_task(Func task_fn)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::packaged_task<void()>(std::move(task_fn)));
        }
        condition.notify_one();
    }

private:
    /// Vector of worker threads.
    std::vector<std::thread> workers;
    /// Queue of tasks waiting to be executed.
    std::queue<std::packaged_task<void()>> tasks;
    /// Mutex for synchronizing access to the task queue.
    std::mutex queue_mutex;
    /// Condition variable for signaling task availability.
    std::condition_variable condition;
    /// Flag indicating whether the ThreadPool should stop processing tasks.
    bool stop;
};