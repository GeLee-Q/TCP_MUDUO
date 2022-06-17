
#include <memory>
#include <sys/epoll.h>

#include <mylib/Channel.h>
#include <mylib/EventLoop.h>


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop * loop, int fd):
    loop_(loop), fd_(fd),
    events_(0), revents_(0),
    index_(-1), tied_(false){}

Channel::~Channel(){

}


// 当channel 所表示的fd的事件改变，
// update将通过eventLoop更改Poller中fd对应的事件
void Channel::update(){
    loop_->updateChannel(this);
}

//在eventLoop中去除这个Channel
void Channel::remove(){
    loop_->removeChannel(this);
}

void Channel::tie(const std::shared_ptr<void> & obj){
    tie_ = obj;
    tied_ = true;
}

void Channel::handleEvent(Timestamp receiveTime){
    if(tied_){
        std::shared_ptr<void> guard = tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime){
    // tcpConnection 对应的Channel通过shutdown关闭写端 epoll触发EPOLLHUP
    if((revents_ & EPOLLHUP) && !(revents_  & EPOLLIN)){
        if(closeCallback_){
            closeCallback_();
        }
    }

    if(revents_ & EPOLLERR){
        if(errorCallback_){
            errorCallback_();
        }
    }

    if(revents_ & (EPOLLIN | EPOLLPRI)){
        if(readCallback_){
            readCallback_(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT){
        if(writeCallback_){
            writeCallback_();
        }
    }
}