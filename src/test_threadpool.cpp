#include <iostream>
#include <string>
#include "tcp/ThreadPool.h"

void print(int a, double b, const char *c, std::string d){
    std::cout << a << b << c << d << std::endl;
}

void test(){
    std::cout << "help" << std::endl;
}

int main(int argc, char const *argv[])
{
    int thread_num = std::thread::hardware_concurrency();
    // std::cout << thread_num << std::endl;
    ThreadPool* poll = new ThreadPool(thread_num);
    std::function<void()> func = std::bind(print, 1, 3.14, "hello", std::string("world"));
    poll->enqueue(func);
    func = test;
    poll->enqueue(func);
    delete poll;
    return 0;
}
