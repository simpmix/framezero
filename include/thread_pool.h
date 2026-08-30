#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <atomic>

namespace FrameZero {

// A cross-platform C++11 Thread Pool for multi-threading heavy engine tasks.
// Works seamlessly on Windows, Linux, and macOS.
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

public:
    ThreadPool(size_t threads = std::thread::hardware_concurrency()) : stop(false) {
        if (threads == 0) threads = 2; // Fallback
        
        for(size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for(;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this] { 
                            return this->stop || !this->tasks.empty(); 
                        });
                        
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task(); // Execute the job
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker : workers) {
            worker.join();
        }
    }

    // Submit a job to the thread pool (e.g., A* Pathfinding, Audio loading)
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
};

} // namespace FrameZero
