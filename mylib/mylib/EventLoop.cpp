#include <cstddef>
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

#include <mylib/EventLoop.h>
#include <mylib/Logger.h>


__thread EventLoop * t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

int creatEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        LOG_FATAL("eventfd error: %d\n", errno);
    }
    return evtfd;
};

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    wakeupFd_(creatEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_DEBUG("EventLoop created %p in thread %d\n", this, threadId_);
    if(t_loopInThisThread){
         LOG_FATAL("Another EventLoop %p exists in this thread %d\n", t_loopInThisThread, threadId_);

    }else{
        t_loopInThisThread = this;
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));

    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disbaleAll(); // channel 移除所有感兴趣的事件
    wakeupChannel_->remove();     // 把Channel 从Evetnloop上删除掉
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping\n", this);

    while(!quit_){
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for(Channel * channel : activeChannels_){
            channel->handleEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping\n", this);
    looping_= false;
}

void EventLoop::quit()
{
    quit_ = true;

    if(!isInLoopThread()){
        wakeUp();
    }
}

// 在当前loop中执行cb
void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread()){
        cb();
    }else{
        queueInLoop(cb);
    }
}

/* 
    unique_lock | lock_guard 符合raii思想
    lock_guard严格在解构时候 unlock
    unique_lock 则可以 提前的释放
    unique_lock额外存储了一个flag是否被释放
 */
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    if(!isInLoopThread() || callingPendingFunctors_){
        wakeUp(); // 唤醒loop所在的线程
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if( n!= sizeof(one)){
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8\n", n);
    }
}


//----------EventLoop -> Poller 的方法

void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const Functor & functor : functors){
        functor();
    }

    callingPendingFunctors_ = false;
}
