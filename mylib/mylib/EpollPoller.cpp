#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <mylib/EpollPoller.h>
#include <mylib/Logger.h>

const int kNew =  -1; // 某个Channel 还没添加到Epoll
const int kAdded = 1; // 某个Channel 已经添加到了Epoll
const int kDeleted = 2; //某个Channel 已经从Poller中被删除了

EpollPoller::EpollPoller(EventLoop * loop)
    :Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(KIntEventListSize) // vector<epoll_event> (16)
{   
    //如果初始化报错，那么报错信息写入日志
    if(epollfd_ < 0){
        LOG_FATAL("epoll_create error: %d  \n", errno);
    }
}

EpollPoller::~EpollPoller(){
    ::close(epollfd_);
}


/* 
 epoll_wait 得到返回的 events[]
 fillactiveChannel 设置  Channel返回的事件类型，设置Channel指向的epoll_date.ptr
 */
Timestamp EpollPoller::poll(int timeoutMs, ChannelList * activeChannels){
    
    LOG_INFO("func=%s => fd total count:%lu\n", __FUNCTION__, channels_.size());

    /* 
    The "events" parameter is a buffer that will contain triggered events. 
    & * events_.begin() 数组的首地址
     */
    int numEvents = ::epoll_wait(epollfd_,
                     & * events_.begin(), 
                    static_cast<int>(events_.size()), 
                     timeoutMs);

    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0){
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        //扩容
        if(numEvents == events_.size()){
            events_.resize(events_.size() * 2);
        }
    }else if(numEvents == 0){
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }else{
        // EINTR说明有中断发生
        if(saveErrno != EINTR){
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() error!");
        }
    }
    return now;
}

void EpollPoller::update(int operation, Channel * channel){
    epoll_event event;
    ::memset(&event, 0, sizeof(event));

    int fd = channel->fd();

    // 从用户态传输到内核态
    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    //处理epoll_ctl 的错误信息
    if(::epoll_ctl(epollfd_, operation, fd, &event) <0){
        if(operation == EPOLL_CTL_DEL){
            LOG_ERROR("epoll_ctl del error: %d \n", errno);
        }else{
            LOG_ERROR("epoll_ctl add/mod error:%d \n", errno);
        }
    }


}

void EpollPoller::updateChannel(Channel *channel){
    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);

    if(index == kNew || index == kDeleted){
        //如果是新的链接，存入Map中
        if(index == kNew){
            int fd = channel->fd();
            channels_[fd] = channel;
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);

    }else{
        int fd = channel->fd();
        // 如果没事件，删除
        if(channel->isNoneEvent()){
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }else{
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

// 从Poller中删除channel
void EpollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd); //用户态保存的信息 去除map<fd, channel>的信息

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);

    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 填写活跃的连接
// 注册时候， 会对 epoll_events.data.ptr 中注册一个Channel
// Channel上绑定了回调函数
void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {   
        // 从内核态传出到用户态的内存空间
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); 
        // EventLoop就拿到了它的Poller给它返回的所有发生事件的channel列表了
    }
}