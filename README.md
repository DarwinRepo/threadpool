# threadpool
A simple C++11 Thread Pool implementation.
Basic usage:

    // create thread pool with 4 worker threads

    ThreadPool pool(4);

    // enqueue and store future

    auto result = pool.enqueue([](int answer) { return answer; }, 42);

    // get result from future

    std::cout << result.get() << std::endl;


A dynamic load thread pool implemented using C++11, the main features:

Threads are preset, and the number of threads is dynamically and automatically adjusted according to the load.

Automatically create new threads when necessary, assuming the current task has more than the number of threads.

Automatically recycle idle threads when necessary, based on a certain period of idleness.

Provides the same usage as the standard library async.


使用 C++11 实现的动态负载线程池，主要特性：

预先设定线程，并根据负载动态自动调整线程数量。

必要时自动创建新线程，假设当前任务多出线程数量。

必要时自动回收空闲的线程，基于一定时间的空闲。

提供与标准库async同样的使用方式。

    #include <iostream>
    #include "windows.h"
    #include "threadpool.hpp"

    int async(int i){
        while (i--)
        {
            std::cout << i;
            Sleep(1000);
        }
        return 1;
    }

    class myclass
    {
    public:
      void async() { return; }
      int async1(int i) { return i; }
    };

    int main()
    {     
        threadpool my(5);

        //demo A for lemda
        my.async([] { return 0; });

        //demo B for class member function
        myclass mycl;
        my.async(&myclass::async, mycl);

        //demo C for class member function have return value
        auto y = my.async(&myclass::async1, mycl , 1);
        auto result = y.get();

        //demo D for dynamic create and recovery
        int i = 9;   
        while (i--)
        {
            my.async(async, i); //if you want get result , please demo E
            Sleep(100);
        } 

        //demo E for dynamic create and recovery ,and get result
        i = 9;
        std::future<int> fu[9];
        while (i--)
        {
            fu[i] =  my.async(async, i); //if you want get result , please demo E
            Sleep(100);
        }

        i = 9;
        int res[9];
        while (i--)
        {
            res[i] = fu[i].get();
        }
        //demo F for ~threadpool
    }
