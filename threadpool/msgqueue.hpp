#ifndef _H_MSGQUEUE_H_  
#define _H_MSGQUEUE_H_ 

#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>

template<typename Msg>
class msgqueue
{
public:
	void add(Msg&& msg)	{
		std::unique_lock<std::mutex> lock(guard_mutex);
		msgs.push(msg);
		lock.unlock();
		signal.notify_one();
	}
	Msg get() {
		std::unique_lock<std::mutex> lock(guard_mutex);
		while (msgs.empty()) {
			signal.wait(lock);
		}
		auto msg = msgs.front();
		msgs.pop();
		return msg;
	}
private:
	std::queue<Msg> msgs;
	mutable std::mutex guard_mutex;
	std::condition_variable signal;
};

#endif