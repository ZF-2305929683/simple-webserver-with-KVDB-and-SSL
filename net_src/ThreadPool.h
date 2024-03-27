#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include "Macros.h"

class ThreadPool
{
private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex tasks_mtx_;
    std::condition_variable cv_;
    bool stop_;

public:
    explicit ThreadPool(int size = 10);
    ~ThreadPool();

    DISALLOW_COPY_AND_MOVE(ThreadPool);

    template<typename F,typename... Args>
    auto add(F&& f, Args&&... args)
    ->std::future<decltype(f(args...))>;
    
};

template<typename F,typename... Args>
auto ThreadPool::add(F&& f, Args&&... args)
->std::future<decltype(f(args...))>
{
    using return_type = decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<return_type()>>
    (
        std::bind(std::forward<F>(f),std::forward<Args>(args)...)
    );
    std::future<return_type> res = task->get_future();  


    std::unique_lock<std::mutex> lock(tasks_mtx_);
    if(stop_) throw std::runtime_error("enqueue on stopped ThreadPool");
    tasks_.emplace([task](){ (*task)(); });  
    
    cv_.notify_one();    
    return res; 
}