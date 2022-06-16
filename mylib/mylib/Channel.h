#pragma once

#include <functional>
#include <memory>

#include <mylib/nocopyable.h>
#include <mylib/Timestamp.h>

class EventLoop;

/*
   EventLoop, Channel, Epoll 
   将文件描述符封装成一个Channel类 
   封装 sockfd 和相关的 event 
   相关时间的回调函数
*/


class Channel : noncopyable{
private:

public:
    using EventCallback = std::function<void()>;
    using EventReadback = std::function<void(Timestamp)>;


};