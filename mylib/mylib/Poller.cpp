#include <mylib/Channel.h>
#include <mylib/Poller.h>

Poller::Poller(EventLoop * loop)
        : ownerLoop_(loop)   
{}

bool Poller::hasChannel(Channel *channel) const{
    auto  it = channels_.find(channel->fd());

    if(it == channels_.end() ){
        return false;
    }

    return it->second == channel;
}