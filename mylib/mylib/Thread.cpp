#include <semaphore.h>

#include <mylib/Thread.h>
#include <mylib/CurrentThread.h>

Thread::Thread(ThreadFunc func, const std::string &name)
        : started_(false),
        joined_(false),
        tid_(0),
        func_(std::move(func)),
        name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_){
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread(
        [&](){
            tid_ = CurrentThread::tid();
            sem_post(&sem);
            func_();
        }
    ));

    // 必须阻塞， 获取上面新创建线程的tid值
    sem_wait(&sem);
    
}


// 让线程分离， 使用join时，主线程阻塞。 detach 主线程失去线程控制权
void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty()){
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread %d", num);
        name_ = buf;
    }
}