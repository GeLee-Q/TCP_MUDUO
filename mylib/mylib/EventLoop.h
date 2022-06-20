#pragma once

#include <algorithm>
#include <ctime>
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

#include <mylib/noncopyable.h>
#include <mylib/Timestamp.h>
#include <mylib/CurrentThread.h>
#include <mylib/Poller.h>
#include <mylib/Channel.h>


class Channel;
class Poller;

class EventLoop: noncopyable{

public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    
    // start events loop
    void loop();

    // quit events loop
    void quit();

    Timestamp pollReturnTime() const {
        return pollReturnTime_; // q1;
    }


/* 
    runInloop | queueInloop 非常重要
    
 */
    // execute in current loop
    void runInLoop(Functor cb);

    // 把上层注册的回调函数 cb 放入队列中，唤醒Loop所在的线程执行Cb
    void queueInLoop(Functor cb);

/* 
    Reactor -> SubReactor() 实现高并发
    mainReactor 处理接受链接的请求
    SubReactor 处理事件
    wakeUp 在epoll_wait 后解除阻塞
 */
    //通过eventfd唤醒loop所在的线程
    void wakeup();

    //EventLoop 调用Poller的方法
    void updateChannel(Channel * channel);
    void removeChannel(Channel * channel);
    bool hasChannel(Channel * channel);


    //判断EventLoop对象是否在自己的线程里
    // 但是我可以在其他的线程里调用这个ELoop
    bool isInLoopThread() const{
        return threadId_ == CurrentThread::tid(); 
        // threadId_ 为EventLoop 创建时的线程id 
        // tid 为当前线程的id
    }

private:
    //给eventfd返回的文件描述符wakeupFd_ 绑定的事件回调
    //当wakeup()有事情发生时 调用handleRead 读wakeupFd_ 同时唤醒阻塞的epoll_wait
    void handleRead();
    void doPendingFunctors(); // 执行上层回调

    using ChannelList = std::vector<Channel *>;

    std::atomic_bool looping_;
    std::atomic_bool quit_;

    const pid_t threadId_; // 记录当前线程是被哪个线程创建的

    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;

    /* 
    当mainloop 获取一个新用户的Channel 通过轮询选择一个subLoop
    通过该成员唤醒subloop来处理Channel
     */
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    //返回Poller检测到的当前有事件发生的Channel
    ChannelList activeChannels_;

    // 标识当前loop是否需要执行回调操作
    std::atomic_bool callingPendingFunctors_;


    //存储loop需要执行的回调函数
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_; // 保护pendingFunctors_的多线程安全
};