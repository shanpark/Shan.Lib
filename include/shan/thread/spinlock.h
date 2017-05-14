//
//  spinlock.h
//  Shan.Thread
//
//  Created by Sung Han Park on 2017. 5. 13..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_thread_spinlock_h
#define shan_thread_spinlock_h

#include <shan/object.h>

namespace shan {
namespace thread {

/**
 기대되는 평균 waiting 시간이 thread context switching 시간보다 짧을 때만 이용한다.
 무한 대기 상태가 있을 수 있는데 spinlock을 이용하면 대기하는 thread는 대기하는 동안 CPU core를 
 full로 사용하게 된다.
 */
class spinlock : public object {
public:
	spinlock() : _l(false) {}

	void lock() noexcept {
		while (_l.test_and_set(std::memory_order_acquire))
			; // spin
	}

	void unlock() noexcept {
		_l.clear(std::memory_order_release);
	}

private:
	std::atomic_flag _l;
};

} // namespace thread
} // namespace shan

#endif /* shan_thread_spinlock_h */
