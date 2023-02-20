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
    threadpool my(5);
    int i = 9;   
    while (i--)
    {
        my.async(async, i);
        Sleep(100);
    } 
}

