#include <mylib/Channel.h>
#include <mylib/Poller.h>

Poller::Poller(EventLoop * loop)
        : ownerLoop_(loop)   
{}

bool Poller::hasChannel(Channel *channel) const{
    auto  it = Channels_.find(channel->fd());

    if(it == Channels_.end() ){
        return false;
    }

    return it->second == channel;
}