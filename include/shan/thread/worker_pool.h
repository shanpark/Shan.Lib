//
//  worker_pool.h
//  Shan.Thread
//
//  Created by Sung Han Park on 2017. 5. 13..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_thread_worker_pool_h
#define shan_thread_worker_pool_h

#include <functional>
#include <future>
#include <vector>

#include <shan/object.h>
#include <shan/thread/spinlock_queue.h>

namespace shan {
namespace thread {		

class worker_pool : public object {
	using work_t = std::function<void()>;

public:
	worker_pool(std::size_t worker_count)
	: _worker_count(worker_count), _running(true), _wait_stop(false), _working_threads(0) {
		for (std::size_t inx = 0 ; inx < _worker_count ; inx++) {
			_workers.emplace_back(std::thread([this](){
				do_work();
			}));
		}
	}

	std::future<void> schedule(work_t work) {
		if (_running) {
			if (_worker_count > 0) {
				std::packaged_task<void()> task(work);
				auto f = task.get_future();
				_task_queue.push(std::move(task));
				{
					std::lock_guard<std::mutex> lock(_mutex);
					_cv.notify_one();
				}
				return f;
			}
			else {
				work();
				return std::future<void>();
			}
		}
		return std::future<void>();
	}

	void wait_complete() {
		if (_worker_count > 0) {
			std::unique_lock<std::mutex> lock(_complete_mutex);
			_complete_cv.wait(lock, [this](){ return (_working_threads == 0); });
		}
	}

	void stop(bool wait) {
		_wait_stop = wait;
		_running = false;

		if (_worker_count > 0) {
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_cv.notify_all();
			}

			for (std::thread& t : _workers) {
				if (t.joinable())
					t.join();
			}
		}
	}

	void do_work() {
		std::packaged_task<void()> task;

		while (_running || _wait_stop) {
			if (_task_queue.pop(task)) {
				++_working_threads;
				task();
				if ((--_working_threads == 0) && _task_queue.empty()) {
					std::lock_guard<std::mutex> lock(_complete_mutex);
					_complete_cv.notify_all();
				}
			}
			else {
				std::unique_lock<std::mutex> lock(_mutex);

				if (!_running && _wait_stop) // running은 아니고 wait_stop 상태인데 queue가 비었다면 loop를 빠져나가도록 한다.
					break;
				_cv.wait(lock, [this](){ return !_task_queue.empty() || !_running; });
			}
		}
	}

private:
	const std::size_t _worker_count;
	std::atomic_bool _running;
	std::atomic_bool _wait_stop;
	std::atomic_int _working_threads;
	std::mutex _mutex;
	std::mutex _complete_mutex;
	std::condition_variable _cv;
	std::condition_variable _complete_cv;
	spinlock_queue<std::packaged_task<void()>> _task_queue;
	std::vector<std::thread> _workers;
};

} // namespace thread
} // namespace shan

#endif /* shan_thread_worker_pool_h */
