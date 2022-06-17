#include <iostream>

#include <mylib/Logger.h>
#include <mylib/Timestamp.h>

//输出唯一的实例对象 单例

Logger & Logger:: instance(){
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int level){
    logLevel_ = level;
}

void Logger::log(std::string msg){
    switch(logLevel_){
    case INFO:
        std::cout << "[INFO]";
    case ERROR:
        std::cout << "[ERRIR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    default:
        break;    
    }

    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}