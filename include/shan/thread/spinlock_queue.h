//
//  spinlock_queue.h
//  Shan.Thread
//
//  Created by Sung Han Park on 2017. 5. 13..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_thread_spinlock_queue_h
#define shan_thread_spinlock_queue_h

#include <deque>
#include <mutex>
#include <shan/object.h>
#include <shan/thread/spinlock.h>

namespace shan {
namespace thread {
		
template<typename T>
class spinlock_queue : public object {
public:
	bool empty() {
		std::lock_guard<spinlock> lock(_sl);
		return _queue.empty();
	}

	void push(const T& val) {
		std::lock_guard<spinlock> lock(_sl);
		_queue.push_back(val);
	}
	void push(T&& val) {
		std::lock_guard<spinlock> lock(_sl);
		_queue.push_back(std::forward<T>(val));
	}

	bool pop(T& val) {
		std::lock_guard<spinlock> lock(_sl);
		if (_queue.empty())
			return false;

		val = std::forward<T>(_queue.front());
		_queue.pop_front();
		return true;
	}

private:
	spinlock _sl;
	std::deque<T> _queue;
};

} // namespace thread
} // namespace shan

#endif /* shan_thread_spinlock_queue_h */
