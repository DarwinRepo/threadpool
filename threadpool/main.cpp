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

