# threadpool

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
