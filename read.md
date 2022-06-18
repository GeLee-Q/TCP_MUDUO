# my_muduo



## CMake 工程构建

- src 源码文件
- mylib muduo网络文件
- 顶层CMakeLists.txt 构造项目





## mylib

### 辅助类：

#### nocopyable

- 通过继承的方式来让接下来的类表示不可拷贝。

#### CheckError | Logger

- 检查错误信息
- 错误信息处理 写入日志类

#### Timestamp

- 更新事件

#### 

### 顶层设计类：

#### TcpServer 

​			

#### EventLoop



### Epoll类：

#### Poller

纯虚基类

`unordered_map<fd, Channel>` 存储文件描述符和Channel的关系；

#### EpollPoller

- 在Poller的基础上实现工程化的Epoll相关功能
- 实现用户态Channel 和 内核态epoll的交互功能
- `epoll_events.data.ptr == channel`
- `epoll_events.events = channel->events()`
- `epoll_events.data.fd = channnel->fd`

#### Channel

-  Channel 封装了 `sockfd` 和相关的 `event` 
- Channel通过EventLoop调用Poller 进行update

-   通过 EventLoop 调用Channel 执行事件

-   二者通过EventLoop 沟通

​          Channel <-- EventLoop --> Epoll

### 网络类 && 连接类 

#### InetAddress



####  Socket



#### Connection

​	Channel 代表 sockfd 和相关的 event 

-    通过 Epoll 返回文件描述符

-   通过 EventLoop 调用Channel 执行事件

-   二者通过EventLoop 沟通

​           Channel <-- EventLoop --> Epoll

#### Acceptor



#### Buffers



### 线程池类

#### Thread



#### 