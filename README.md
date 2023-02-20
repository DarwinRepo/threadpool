# threadpool

A dynamic load thread pool implemented using C++11, the main features:

Threads are preset, and the number of threads is dynamically and automatically adjusted according to the load.
Automatically create new threads when necessary, assuming the current task has more than the number of threads.
Automatically recycle idle threads when necessary, based on a certain period of idleness.
Provides the same usage as the standard library async.
