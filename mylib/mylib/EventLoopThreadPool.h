#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>

#include <mylib/noncopyable.h>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    
    EventLoopThreadPool(EventLoop * baseloop, const std::string & nameArg);
    ~EventLoopThreadPool();
  
    //构造函数时调用
    void start(const ThreadInitCallback & cb = ThreadInitCallback());
    
    // 工作在多线程模式中，baseLoop_ 会默认以轮询的方式分配channel给subloop
    EventLoop * getNextLoop();

    std::vector<EventLoop *> getAllLoops();

    void setThreadNum(int numThreads){
        numThreads_ = numThreads; 
    }


    bool started() const { return started_;}
    const std::string name() const { return name_ ;}

private:
    // 如果线程数为 1， 直接使用当前eventloop 否则创建多eventloop
    EventLoop * baseloop_; 
    int next_; // 轮询的下标

    int numThreads_;
    bool started_;
    std::string name_;

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};