#pragma once

#include <vector>
#include <sys/epoll.h>


#include <mylib/Timestamp.h>
#include <mylib/Channel.h>
#include <mylib/Poller.h>

class Channel;

class EpollPoller : public Poller{
public:
    EpollPoller(EventLoop * loop);
    ~EpollPoller() override;

    // 重写基类的纯虚方法
    Timestamp poll(int timeoutMs, ChannelList * activeChannels) override;
    void updateChannel(Channel * channel) override;
    void removeChannel(Channel * channel) override;

private:
    static const int KIntEventListSize = 16;

    //填写活跃的链接
    void fillActiveChannels(int numEvents, ChannelList * activeChannels) const;

    void update(int operation, Channel * channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;        // epoll_create 创建返回的fd保存在epollfd_中
    EventList events_;   // 用于存放epoll_wait 返回所有事件的文件描述夫
    
};