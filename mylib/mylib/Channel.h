#pragma once

#include <functional>
#include <memory>

#include <mylib/noncopyable.h>
#include <mylib/Timestamp.h>

class EventLoop;

/* Channel 代表 sockfd 和相关的 event 
   
   通过 Epoll 返回文件描述符
   通过 EventLoop 调用Channel 执行事件
   二者通过EventLoop 沟通
   Channel <-- EventLoop --> Epoll
*/


class Channel : noncopyable{
public:
    // 两个回调函数对象
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop * loop , int fd);
    ~Channel();

    // fd得到Epoll通知后 处理事件 handleEvent 在Loop中调用
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数
    void setReadCallback(ReadEventCallback cb){
        readCallback_ = std::move(cb);
    }

    void setWriteCallback(EventCallback cb){
        writeCallback_ = std::move(cb);
    }

    void setCloseCallback(EventCallback cb){
        closeCallback_ = std::move(cb);
    }

    void setErrorCallback(EventCallback cb){
        errorCallback_ = std::move(cb);
    }
    
    // life-period control
    void tie(const std::shared_ptr<void> &);


    // 得到文件描述符和关心事件的类型；
    int fd() const{
        return fd_;
    }

    int events() const {
        return events_;
    }

    void set_revents(int revt){
        revents_ = revt;
    }

    // 设置fd相应事件状态 update 相当于 epoll_ctl add delelte
    void enableReading(){ events_ |= kReadEvent; update();}
    void disableReading(){ events_ &= ~kReadEvent; update();}

    void enableWriting(){events_ |= kWriteEvent; update(); }
    void disableWriting(){events_ &= ~kWriteEvent; update();}

    void disbaleAll(){ events_ &= ~kNoneEvent; update();}

    //返回fd当前事件的状态
    bool isNoneEvent() const { return events_ == kNoneEvent ;}

    bool isWriting() const {return events_ & kWriteEvent; }
    bool isReading() const {return events_ & kReadEvent;}

    int index() const { return index_; }
    void set_index(int idx){
        index_ = idx;
    }

    // one loop per thread
    EventLoop * ownerloop() {
        return loop_;
    }
    void remove();

private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop * loop_;  // 事件循环
    const int fd_;      // Epoll 监听的文件描述符
    int events_;        // fd 感兴趣的事件
    int revents_;       // Epoll返回的具体事件
    int index_;
    /* 用于查看当前文件描述符是否在内核态的epoll中
    index_ = -1 Channel 没有注册到内核态的Epoll
           =  1 Channel 已经注册到内核态
           =  2 Channel 在Epoll中已经被删除了
     */


    // 改变弱回调的生命周期的问题
    std::weak_ptr<void> tie_;
    bool tied_;


    //负责具体事件的回调
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};