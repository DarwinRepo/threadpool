#ifndef _H_THREADPOOL_H_  
#define _H_THREADPOOL_H_ 

#include <cassert>
#include <future>
#include <functional>
#include <unordered_map>
#include <queue>
#include <atomic>

enum class  task_type_e{
	none = 0,
	remove,
	create
};

template<typename _Msgty>
class msgqueue {
public:
	void add(_Msgty&& _msg) {
		std::unique_lock<std::mutex> lock(guard_mutex);
		msgs.push(_msg);
		lock.unlock();
		signal.notify_one();
	}
	_Msgty get() {
		std::unique_lock<std::mutex> lock(guard_mutex);
		while (msgs.empty()) {
			signal.wait(lock);
		}
		auto msg = msgs.front();
		msgs.pop();
		return msg;
	}
private:
	std::queue<_Msgty> msgs;
	mutable std::mutex guard_mutex;
	std::condition_variable signal;
};

class threadpool {
public:
	threadpool(unsigned int _min_thread = 1, unsigned int _outtime_ms = 2000, unsigned int _max_thread = 0x10)
		:min_thread{ _min_thread },
		max_thread{ _max_thread },
		outtime_ms{ _outtime_ms },
		idle_thread{ _min_thread },
		current_thread{ _min_thread },
		deamon_thread{ &threadpool::deamon_task,this }	{
		while (_min_thread--) {
			std::thread new_thread{ &threadpool::worker,this };
			thread_pool.emplace(new_thread.get_id(), std::move(new_thread));
		}
	}
	~threadpool() {
		is_quit = true;
		signal.notify_all();
		for (auto& it : thread_pool) {
			assert(it.second.joinable());
			it.second.join();
		}
		deamon_tasks.add(std::make_pair(task_type_e::none, std::thread::id()));
		deamon_thread.join();
	}
	template <typename _Fty, typename... _ArgTypes>
	auto async(_Fty&& _Fnarg, _ArgTypes&&... _Args) -> std::future<typename std::result_of<_Fty(_ArgTypes...)>::type> {
		using _Ret = typename std::result_of<_Fty(_ArgTypes...)>::type;
		auto execute = std::bind(std::forward<_Fty>(_Fnarg), std::forward<_ArgTypes>(_Args)...);
		auto task = std::make_shared<std::packaged_task<_Ret()>>(std::move(execute));
		auto result = task->get_future();
		std::lock_guard<std::mutex> lock(guard_mutex);
		tasks.emplace([task] { (*task)(); });
		signal.notify_one();
		return result;
	}
private:
	void worker() {
		while (!is_quit) {
			std::function<void(void)> task;	{
				std::unique_lock<std::mutex> lock(guard_mutex);
				bool is_timeout = !signal.wait_for(lock, std::chrono::milliseconds(outtime_ms), [this] { return is_quit || !tasks.empty(); });
				idle_thread--;
				if (is_quit)return;
				if (is_timeout) {
					if (tasks.empty()) {
						if (current_thread > min_thread && idle_thread != 0) {
							deamon_tasks.add(std::make_pair(task_type_e::remove, std::this_thread::get_id()));
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
					deamon_tasks.add(std::make_pair(task_type_e::create, std::thread::id()));
					current_thread++; idle_thread++;
				}
				task = std::move(tasks.front());
				tasks.pop();
			}
			task();
			idle_thread++;
		}
	}
	void deamon_task() {
		while (!is_quit) {
			auto bg_task = deamon_tasks.get();
			switch (bg_task.first)	{
			case task_type_e::none:
				break;
			case task_type_e::remove:
				(*thread_pool.find(bg_task.second)).second.join();
				thread_pool.erase(bg_task.second);
				break;
			case task_type_e::create:
				std::thread new_thread{ &threadpool::worker,this };
				thread_pool.emplace(new_thread.get_id(), std::move(new_thread));
				break;
			}
		}
	}

private:
	const unsigned int min_thread;
	const unsigned int max_thread;
	const unsigned int outtime_ms;
	std::atomic<unsigned int> idle_thread;
	unsigned int current_thread;
	mutable std::mutex guard_mutex;
	std::condition_variable signal;
	std::queue<std::function<void(void)>> tasks{};
	std::atomic<bool> is_quit{ false };
	std::unordered_map<std::thread::id, std::thread> thread_pool;
	std::thread deamon_thread;
	msgqueue<std::pair<task_type_e, std::thread::id> > deamon_tasks;
};

#endif