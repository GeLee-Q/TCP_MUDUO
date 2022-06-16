#pragma once

/*
    继承该类的类，无法进行拷贝构造和赋值构造
*/

class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable & operator=(const noncopyable & ) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};