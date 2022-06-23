#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include <mylib/Acceptor.h>
#include <mylib/Logger.h>
#include <mylib/InetAddress.h>

static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK| SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err: %d\n", __FILE__, __FUNCTION__,__LINE__,errno);

    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop * loop, const InetAddress & listenAddr, bool reuseport)
        :loop_(loop),
        acceptSocket_(createNonblocking()),
        acceptChannel_(loop, acceptSocket_.fd()),
        listening_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);

    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));

}


Acceptor::~Acceptor()
{
    acceptChannel_.disbaleAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        if (NewConnectionCallback_)
        {
            NewConnectionCallback_(connfd, peerAddr); // 轮询找到subLoop 唤醒并分发当前的新客户端的Channel
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfd reached limit\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}