#include <cstdio>
#include <cstdlib>

#include <mylib/CheckError.h>

// void perror(const char * str) 打印错误的信息
// exit 函数 退出程序 
void ErrorIf(bool condition, const char *msg){
    if(condition){
        perror(msg); 
        exit(EXIT_FAILURE);
    }
}