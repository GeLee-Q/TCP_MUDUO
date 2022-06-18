#include "mylib/Logger.h"
#include "mylib/Poller.h"
#include <cstddef>
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

#include <mylib/EventLoop.h>


__thread EventLoop * t_loopInThisThread = nullptr;

const int kPollTimems = 10000;

int creatEventfd(){
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
    weakupFd_(creatEventfd()),
    wakeupChannel_(new Channel(this, weakupFd_))
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