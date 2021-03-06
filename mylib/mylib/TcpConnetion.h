#pragma once

#include <memory>
#include <string>
#include <atomic>


#include <mylib/Callbacks.h>
#include <mylib/Buffer.h>

#include <mylib/noncopyable.h>
#include <mylib/InetAddress.h>
#include <mylib/Timestamp.h>

class Channel;
class EventLoop;
class Socket;

/* 
    TcpServer-> Acceptor -> accept 得到连接的accept
    ->TcpConnection 设置回调函数 -> 设置Channel -> Poller -> Channel回调
 */


/* 
    shared_from_this 
    传入传出自动调整引用计数
    sockfd 接受的是acceptor接受的已连接的文件描述符
 */

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,
                  const std::string & nameArg,
                  int sockfd,
                  const InetAddress & localAddr,
                  const InetAddress & peerAddr);
    
    ~TcpConnection();

    // 获得私有成员信息
    EventLoop * getLoop() const { return loop_; }
    const std::string & name() const { return  name_; }
    const InetAddress & localAddress() const {return localAddr_; }
    const InetAddress & peerAddress() const {return peerAddr_; }

    // 查看连接状态
    bool connected() const { return state_ == kConnected; }

    void send(const std::string &buf);

    void shutdown();


    /* 
        channel中的回调 是读写关闭错误四种文件描述符的回调
        这些回调是EventLoop中的doPendingFunctors()
     */
   void setConnectionCallback(const ConnectionCallback &cb)
    { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb)
    { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    { writeCompleteCallback_ = cb; }
    void setCloseCallback(const CloseCallback &cb)
    { closeCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    void connectEstablished();
    
    void connectDestroyed();

private:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    void setState(StateE state){ state_ = state;}


/* 
    事件的读写关闭错误的回调
    注册到channel上 ， handlEvent
 */
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void * data, size_t len);
    void shutdownInLoop();


    EventLoop * loop_; // 单Reactor 指向baseloop 多Reactor 指向subloop
    const std::string name_;
    std::atomic_int state_;
    bool reading_;   

    // Socket Channel 这里和Acceptor类似    Acceptor => mainloop    TcpConnection => subloop
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    // 回调函数的传递
    /* 
       用户-> TcpServer->TcpConnection->Channel
     */
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;

    // 数据缓冲区
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};