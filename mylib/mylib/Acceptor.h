#pragma once

#include <functional>

#include <mylib/noncopyable.h>
#include <mylib/Socket.h>
#include <mylib/Channel.h>

class EventLoop;
class InetAddress;

class Acceptor: noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;

    Acceptor(EventLoop * loop, const InetAddress & listenAddr, bool reusePort);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback & cb)
    {
        NewConnectionCallback_ = cb;
    }

    bool listenning() const {return listening_;}
    void listen();

private:
    void handleRead();

    EventLoop * loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;

    NewConnectionCallback NewConnectionCallback_;
    bool listening_;
};