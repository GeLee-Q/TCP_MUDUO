#pragma once

/* 
    用户使用muduo库构造的 服务器
 */

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

#include <mylib/EventLoop.h>
#include <mylib/Acceptor.h>
#include <mylib/InetAddress.h>
#include <mylib/noncopyable.h>
#include <mylib/EventLoopThreadPool.h>
#include <mylib/Callbacks.h>
#include <mylib/TcpConnetion.h>
#include <mylib/Buffer.h>

class TcpServer
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    enum Option{
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop * loop,
             const InetAddress & listenAddr,
             const std::string & nameArg,
             Option option = kNoReusePort);
    
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb;}
    void setConnectionCallback(const ConnectionCallback & cb) {connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback & cb) {messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback & cb) { writeCompleteCallback_ = cb;}

    // 设置 subloop的数目
    void setThreadNum(int numThreads);

    void start();

private:

    void newConnection(int sockfd, const InetAddress & peerAddr);
    void removeConnection(const TcpConnectionPtr & conn);
    void removeConnectionInLoop(const TcpConnectionPtr & conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop * loop_;

    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;// 运行在mainloop监听新事件
    std::shared_ptr<EventLoopThreadPool> threadPool_; // one loop per thread


    ConnectionCallback connectionCallback_;
    MessageCallback    messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;

    std::atomic_int started_;

    int nextConnId_;

    ConnectionMap connections_;
};