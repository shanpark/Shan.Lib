//
//  pool.h
//  Shan.Util
//
//  Created by Sung Han Park on 2017. 3. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_util_pool_h
#define shan_util_pool_h

#include <deque>
#include <memory>
#include <limits>
#include <mutex>
#include <cassert>
#include <shan/object.h>

namespace shan {
namespace util {


////////////////////////////////////////////////////////////////////////////////
// poolable interface.
// all pooling objects must implement this interface.
//
template<typename... Args>
class poolable {
public:
	virtual void reset(Args... args) noexcept = 0; // argument list must be the same as a constructor.
};

////////////////////////////////////////////////////////////////////////////////
// static_pool
// provides static public pooling method.
// The poolable object must implement the reset method with the same parameters
// as the constructor.
template<typename T, std::size_t MaxSize = std::numeric_limits<std::size_t>::max()>
class static_pool : public object {
public:
	using ptr_type = std::shared_ptr<T>;

public:
	template<typename... Args>
	static ptr_type get_object(Args&&... args) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_pool.empty()) {
			auto obj = std::make_shared<T>(std::forward<Args>(args)...); // constructor's argument list.
			return obj;
		}
		else {
			auto obj = _pool.front();
			obj->reset(std::forward<Args>(args)...); // The poolable object must implement the reset method with the same parameters as the constructor.
			_pool.pop_front();
			return obj;
		}
	}

	static void return_object(ptr_type obj) noexcept {
		if (obj) {
			try {
				std::lock_guard<std::mutex> lock(_mutex);
				if (_pool.size() < MaxSize)
					_pool.push_back(obj);
			} catch (...) {
				// ignore exception. obj will be released.
			}
		}
	}

private:
	static std::mutex _mutex;
	static std::deque<ptr_type> _pool;
};

template<typename T, std::size_t MaxSize>
std::deque<typename static_pool<T, MaxSize>::ptr_type> static_pool<T, MaxSize>::_pool;

template<typename T, std::size_t MaxSize>
std::mutex static_pool<T, MaxSize>::_mutex;

////////////////////////////////////////////////////////////////////////////////
// pool
// provides general pooling method. (not a static class. need to create an object.)
// The poolable object must implement the reset method with the same parameters
// as the constructor.
template<typename T, std::size_t MaxSize = std::numeric_limits<std::size_t>::max()>
class pool : public object {
public:
	using ptr_type = std::shared_ptr<T>;

public:
	template<typename... Args>
	ptr_type get_object(Args&&... args) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_pool.empty()) {
			auto obj = std::make_shared<T>(std::forward<Args>(args)...); // constructor's argument list.
			return obj;
		}
		else {
			auto obj = _pool.front();
			obj->reset(std::forward<Args>(args)...); // The poolable object must implement the reset method with the same parameters as the constructor.
			_pool.pop_front();
			return obj;
		}
	}

	void return_object(ptr_type obj) noexcept {
		try {
			std::lock_guard<std::mutex> lock(_mutex);
			if (_pool.size() < MaxSize)
				_pool.push_back(obj);
		} catch (...) {
			// ignore exception. obj will be released.
		}
	}

protected:
	std::mutex _mutex;
	std::deque<ptr_type> _pool;
};

} // namespace util
} // namespace shan

#endif /* shan_util_pool_h */
