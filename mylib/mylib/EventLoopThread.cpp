#include <mylib/EventLoop.h>
#include <mylib/EventLoopThread.h>

EventLoopThread::EventLoopThread(const ThreadInitCallback & cb,
                                const std::string & name)
    :loop_(nullptr),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),
    mutex_(),
    cond_(),
    callback_(cb) // 线程初始化的callback
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr){
        loop_->quit();
        thread_.join();
    }
}


/* 
    返回真实的线程对象的eventloop的指针
 */
EventLoop * EventLoopThread::startLoop()
{
    thread_.start(); // 启动一个线程
    EventLoop * loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr){
            cond_.wait(lock); // 条件变量阻塞在这里
        }
        loop = loop_;
    }
    return loop;
}

// 给绑定的thread设置回调函数
void EventLoopThread::threadFunc()
{
    EventLoop loop;// 与线程中绑定在一起 与子reactor 绑定在一起
    
    // 线程初始化需要执行的东西
    if(callback_){
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();

    // 当前的eventloop要关闭了
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}