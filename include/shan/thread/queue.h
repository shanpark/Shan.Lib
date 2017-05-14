//
//  queue.h
//  Shan.Thread
//
//  Created by Sung Han Park on 2017. 5. 12..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_thread_queue_h
#define shan_thread_queue_h

#include <deque>
#include <mutex>
#include <condition_variable>
#include <shan/object.h>

namespace shan {
namespace thread {

template<typename T>
class queue : public object {
public:
	std::size_t size() {
		std::lock_guard<std::mutex> lock(_mutex);
		return _queue.size();
	}

	void push(const T& elem) {
		std::lock_guard<std::mutex> lock(_mutex);
		_queue.push_back(elem);
		_cv.notify_one();
	}

	T pop() {
		std::unique_lock<std::mutex> lock(_mutex);
		if (_queue.empty())
			_cv.wait(lock, [this](){ return !_queue.empty(); });

		T ret = _queue.front();
		_queue.pop_front();
		return ret;
	}

	T pop_if_possible() {
		std::lock_guard<std::mutex> lock(_mutex);

		if (_queue.empty())
			return nullptr;

		T ret = _queue.front();
		_queue.pop_front();
		return ret;
	}

	void clear() {
		std::lock_guard<std::mutex> lock(_mutex);
		return _queue.clear();
	}

private:
	std::mutex _mutex;
	std::condition_variable _cv;
	std::deque<T> _queue;
};

} // namespace thread
} // namespace shan

#endif /* shan_thread_queue_h */
