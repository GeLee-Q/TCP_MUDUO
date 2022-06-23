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

// wakeupFd用来notify唤醒subReactor处理新来的Channel
int creatEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        LOG_FATAL("eventfd error: %d\n", errno);
    }
    return evtfd;
};

/* 
    threadId_ 返回创建eventloop的线程pid
    sm_pointer 构造唯一的poller_ 联系EventLoop 与 Poller
    wakeupFd_
    sm_pointer wakeupChannel 调用Channel的构造函数 和wakeupfd_绑定到一起
 */
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
    wakeupChannel_->disbaleAll(); // channel 移除所有感兴趣的事件 不关心任何时间
    wakeupChannel_->remove();     // 把Channel 从Evetnloop上删除掉 
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping\n", this);

    /* 
    poller_->poll 返回活跃的Channel
    然后通知Channel执行事件 
     */
    while(!quit_){
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for(Channel * channel : activeChannels_){
            // 执行当前文件描述符对应的回调函数
            channel->handleEvent(pollReturnTime_);
        }
        /* 
            执行当前EventLoop 事件循环需要处理的回调函数
            accepctor -> connfd 打包为Channel -> TcpServer::connection 通过轮询将链接对象分配给sunloop处理
            mainloop调用queueInloop 将回调加入subloop
         */
        doPendingFunctors();
        // 执行完之后会回到epoll_wait的阻塞当中，剩下的上层回调难以执行
    }

    LOG_INFO("EventLoop %p stop looping\n", this);
    looping_= false;
}

void EventLoop::quit()
{
    quit_ = true;

    if(!isInLoopThread()){
        // 解除poll的阻塞
        wakeup();
    }
}


/* 
  key: 上层如何向这个eventLoop注册事件的
    如果是单reactor runInloop 可以直接调用
    如果是多reactor 通过queueInLoop 下发回调
 */
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

    //如果正在执行上层回调，要进行wakeup操作 去唤醒epoll_wait
    if(!isInLoopThread() || callingPendingFunctors_){
        wakeup(); // 唤醒loop所在的线程
    }
}


// wakeup的回调函数 
void EventLoop::handleRead()
{
    uint64_t one = 1; // 8字节
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if( n!= sizeof(one)){
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8\n", n);
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
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
        //将队列中存在funtors换出，减少加锁临界区的长度，避免锁的开销影响
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const Functor & functor : functors){
        functor(); // 执行当前loop需要执行回调函数
    }

    callingPendingFunctors_ = false;
}
