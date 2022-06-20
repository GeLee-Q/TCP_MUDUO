#pragma once



#include <vector>
#include <unordered_map>

#include <mylib/noncopyable.h>
#include <mylib/Timestamp.h>
#include <mylib/Channel.h>

class Channel;
class EventLoop;

class Poller{
public:
    using ChannelList = std::vector<Channel *>;
    Poller(EventLoop * loop);
    virtual ~Poller() = default;

    // 纯虚函数，此类为抽象类，不可以被实例化，必须通过派生类来进行重写
    virtual Timestamp poll(int timeoutMs, ChannelList * activeChannels) = 0;
    virtual void updateChannel(Channel * channel) = 0;
    virtual void removeChannel(Channel * channel) = 0;

    //判断当前Channel是否在Poller中
    bool hasChannel(Channel * channel) const;

    //eventloop 可以用过该接口获得默认的IO复用的具体实践
    //static修饰 没有this指针，不能访问非static数据成员
    static Poller * newDefaultPoller(EventLoop * loop);

protected:
    //map 存储epoll中注册过的文件描述符 和 Channel对象
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;

private:
    EventLoop * ownerLoop_; // 定义Poller所属于的事件循环
};