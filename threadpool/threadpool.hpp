#ifndef _H_THREADPOOL_H_  
#define _H_THREADPOOL_H_ 

#include <future>
#include <functional>
#include <unordered_map>
#include <queue>
#include <atomic>
#include "msgqueue.hpp"


class threadpool
{
public:
	threadpool(unsigned int _min_thread,  unsigned int _outtime_ms = 2000, unsigned int _max_thread = 256)
		:min_thread{ _min_thread }, max_thread{ _max_thread }, outtime_ms{ _outtime_ms }, idle_thread{ _min_thread  }, current_thread{ _min_thread },
		demon_thread{ &threadpool::demon_task,this }
	{
		while (_min_thread--) {
			std::thread new_thread{ &threadpool::worker,this };
			thread_pool.emplace(new_thread.get_id(), std::move(new_thread));
		}
	}
	~threadpool() { }
	template<typename Func,typename... Param>
	auto async(Func&& func, Param &&... params) -> std::future<typename std::result_of<Func(Param...)>::type>
	{
		auto execute = std::bind(std::forward<Func>(func), std::forward<Param>(params)...);
		auto task = std::make_shared<std::packaged_task<std::result_of<Func(Param...)>::type>>(std::move(execute));
		auto result = task->get_future();
		std::lock_guard<std::mutex> lock(guard_mutex);
		tasks.emplace([task] { (*task)(); });
		signal.notify_one();
		return result;
	}
private:
	void x(){  };
	void worker()
	{
		while (!is_quit)
		{
			std::function<void(void)> task;
			{
				std::unique_lock<std::mutex> lock(guard_mutex);
				bool is_timeout = !signal.wait_for(lock, std::chrono::milliseconds(outtime_ms ), [this] { return is_quit || tasks.empty(); });
				idle_thread--;
				if (is_timeout)	{
					if (is_quit)return;
					if (tasks.empty()) {
						if (current_thread > min_thread) {							
							deamon_tasks.add(std::make_pair(true, std::this_thread::get_id()));
							current_thread--;
							break;
						}
						else {
							idle_thread++;
							continue;
						}
					}
				}
				if (idle_thread == 0 && current_thread < max_thread) {
					deamon_tasks.add(std::make_pair(false, std::this_thread::get_id()));
					current_thread++; idle_thread++; 
				}
				task = std::move(tasks.front());
				tasks.pop();
			}
			task();
			idle_thread++;
		}
	}
	void demon_task()
	{
		while (!is_quit) {
			auto bg_task = deamon_tasks.get();
			if (bg_task.first) {
				(*thread_pool.find(bg_task.second)).second.join();
				thread_pool.erase(bg_task.second);
			}
			else {
				std::thread new_thread{ &threadpool::worker,this };
				thread_pool.emplace(new_thread.get_id(),std::move(new_thread));
			}
		}
	}

private:
	const unsigned int min_thread;
	const unsigned int max_thread;
	const unsigned int outtime_ms;
	unsigned int idle_thread;
	unsigned int current_thread;
	mutable std::mutex guard_mutex;
	std::condition_variable signal;
	std::queue<std::function<void(void)>> tasks{};	
	bool is_quit{ false };
	std::unordered_map<std::thread::id, std::thread> thread_pool;
	std::thread demon_thread;
	msgqueue<std::pair<bool, std::thread::id> > deamon_tasks;
};

#endif