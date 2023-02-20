#include <iostream>
#include "windows.h"
#include "threadpool.hpp"

int async(int i)
{
    while (i--)
    {
        std::cout << i;
        Sleep(1000);
    }
    return 1;
}

int main()
{
    threadpool my(9);
    int i = 10;   
    i = 10;
    while (i--)
    {
        my.async(async, i);
        my.async(async, i);
        Sleep(100);
    }

    std::cout << "Hello World!\n";    
}

