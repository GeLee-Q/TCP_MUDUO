#pragma once

#include <functional>
#include <thread>
#include <memory>
#include <string>
#include <atomic>

#include <mylib/noncopyable.h>


/* 
    创建一个线程，设置线程的名称，启动一个线程
 */
class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string & name = std::string());
    ~Thread();

    void start();
    void join();

    bool started(){
        return started_;
    }

    pid_t tid() const {
        return tid_;
    }

private:
    void setDefaultName();
    // 线程是否启动
    bool started_;
    bool joined_;

    std::shared_ptr<std::thread> thread_;

    pid_t tid_;
    ThreadFunc func_; // 线程回调函数
    std::string name_;

    // 当前创建的线程数
    static std::atomic_int numCreated_;
};