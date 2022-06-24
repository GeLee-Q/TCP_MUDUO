- [CMake 工程构建](#cmake-工程构建)
- [mylib](#mylib)
  - [辅助类：](#辅助类)
    - [nocopyable](#nocopyable)
    - [CheckError | Logger](#checkerror--logger)
    - [Timestamp](#timestamp)
    - [CurrentThead](#currentthead)
  - [顶层设计类：](#顶层设计类)
    - [TcpServer](#tcpserver)
    - [EventLoop](#eventloop)
  - [Epoll类：](#epoll类)
    - [Poller](#poller)
    - [EpollPoller](#epollpoller)
    - [Channel](#channel)
  - [线程类](#线程类)
    - [Thead](#thead)
    - [EventLoopThread](#eventloopthread)
    - [EventLoopThreadPool](#eventloopthreadpool)
  - [网络类 && 连接类](#网络类--连接类)
    - [InetAddress](#inetaddress)
    - [Socket](#socket)
    - [Connection](#connection)
    - [Acceptor](#acceptor)
    - [Buffers](#buffers)

# CMake 工程构建

- src 源码文件
- mylib muduo网络文件
- 顶层CMakeLists.txt 构造项目





# mylib

## 辅助类：

### nocopyable

- 通过继承的方式来让接下来的类表示不可拷贝。

### CheckError | Logger

- 检查错误信息
- 错误信息处理 写入日志类
- 行为写入日志类

### Timestamp

- 返回事件戳，用于记录时间发生节点

### CurrentThead

- 用于辅助EventLoop实现主从Reactor模式
- 获取线程的pid()



## 顶层设计类：

### TcpServer 

​			

### EventLoop

- 链接Channel 和 Poller

- mian-Reactor 和 sub-Reactor

- `loop()` 由三部分组成：

  - 调用 `Poller->poll()`返回 有事件的`channel`
  - 处理`channel`里的回调函数
  - 处理上层注册的回调函数后陷入阻塞

- `wakeupFd_` 和 `wakeupChannel_`  专门用来唤醒`loop`，防止其不能及时处理上层分发的回调函数

- `runInLoop()` 向subloop塞入分发的回调函数

  `queueInLoop()` 分发回调函数 通过`wakeupFd_`写入一个数据唤醒`loop`

- `doPendingFunctors()`将需要处理的回调函数换出，减少锁的临界区开销

​		同时避免死锁，保证加入的回调可以被迅速的执行

## Epoll类：

### Poller

纯虚基类

`unordered_map<fd, Channel>` 存储文件描述符和Channel的关系；

### EpollPoller

- 在Poller的基础上实现工程化的Epoll相关功能
- 实现用户态Channel 和 内核态epoll的交互功能
- `epoll_events.data.ptr == channel`
- `epoll_events.events = channel->events()`
- `epoll_events.data.fd = channnel->fd`

### Channel

-  Channel 封装了 `sockfd` ，相关的 `event` 和回调函数
-  Channel通过EventLoop调用Poller 进行update

-  通过 EventLoop 调用Channel 执行事件

-  二者通过EventLoop 沟通

​          Channel <-- EventLoop --> Epoll



## 线程类

### Thead

- 基础的Thread类
- 创建一个线程，设置线程的名称，启动一个线程

### EventLoopThread

- 将线程与eventloop绑定在一起

- 构造函数 -> `threadFunc()` : 构造EvnetLoop ，上层回调函数进行线程初始化需要进行的操作，绑定 eventLoop；条件变量唤醒

  ->`startLoop()`启动线程-> 启动loop

### EventLoopThreadPool

- 封装EventLoopThread成一个线程池类、
- 最上层的回调函数，向下注册
- 自动的进行管理
- 主Reactor 负责连接



## 网络类 && 连接类 

### InetAddress



###  Socket



### Connection



### Acceptor



### Buffers



