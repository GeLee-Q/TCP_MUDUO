#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>

#include <mylib/noncopyable.h>
#include <mylib/Thread.h>

class EventLoop;

// 将线程与eventloop绑定在一起
class EventLoopThread: noncopyable
{
public:
    // 上层回调
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback & cb = ThreadInitCallback(),
                    const std::string & name = std::string());
    
    ~EventLoopThread();

    EventLoop * startLoop();
private:
    void threadFunc();

    EventLoop * loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;              // 互斥锁
    std::condition_variable cond_;  // 条件变量
    ThreadInitCallback callback_;   // 初始化的一个函数
};
