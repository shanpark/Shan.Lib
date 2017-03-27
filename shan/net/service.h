//
//  service.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 15..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_service_h
#define shan_net_service_h

namespace shan {
namespace net {

class service : public object {
	friend class channel_context;
protected:
	static const int default_buffer_base_size = 4096;

	service(protocol protocol, std::size_t worker_count, std::size_t buffer_base_size = default_buffer_base_size)
	: _protocol(protocol), _worker_count(worker_count), _buffer_base_size(buffer_base_size)
    , _channel_pipeline_ptr(new channel_pipeline()) {}

public:
	bool add_channel_handler(channel_handler* ch_handler_p) {
		return add_channel_handler(channel_handler_ptr(ch_handler_p));
	}
	
	bool add_channel_handler(channel_handler_ptr ch_handler_ptr) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (!is_running()) {
			_channel_pipeline_ptr->add_handler(std::move(ch_handler_ptr));
			return true;
		}
		
		return false; // if service started, can't add handler.
	}

	virtual void wait_stop() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cv.wait(lock, [this](){ return !is_running(); });
	}

protected:
	bool is_tcp() { return (_protocol == protocol::tcp); }
	virtual bool is_running() = 0;

	void start() noexcept {
		_io_service_ptr = std::unique_ptr<asio::io_service>(new asio::io_service());
		_work_ptr = std::unique_ptr<asio::io_service::work>(new asio::io_service::work(*_io_service_ptr));

		// start worker threads
		for (auto inx = 0 ; inx < _worker_count ; inx++)
			_workers.push_back(std::thread(std::bind<std::size_t(asio::io_service::*)()>(&asio::io_service::run, _io_service_ptr.get())));
	}

	// Once stop() is called, the service object can not be reused.
	void stop() noexcept {
		_io_service_ptr->stop();

		for (auto& t : _workers) {
			if (t.joinable()) {
				try {
					t.join();
				}
				catch (...) {
					// A deadlock exception occurs when calling stop() within an io_service run thread.
					t.detach(); // just detach. io_service is already stopped. to the thread will be terminated.
				}
			}
		}
		_workers.clear();

		// class all channels
		for (auto pair : _channel_contexts)
			pair.second->channel_close();
		_channel_contexts.clear();

		_work_ptr = nullptr; // release ownership
		_io_service_ptr = nullptr; // release ownership.
	}

	const std::vector<channel_pipeline::handler_ptr>& channel_handlers() const { return _channel_pipeline_ptr->handlers(); }

	void fire_channel_exception_caught(channel_context_ptr ch_ctx_ptr, const channel_error& e) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr, e]() {
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().rbegin();
			auto end = channel_handlers().rend();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->exception_caught(ch_ctx_ptr.get(), e);
			} catch (const std::exception& e2) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in exception_caught handler. (") + e2.what() + ")")); // can cause infinite exception...
			}
		});
	}

protected:
	const protocol _protocol;
	const std::size_t _worker_count;
	const std::size_t _buffer_base_size;

	std::unique_ptr<asio::io_service> _io_service_ptr;
	std::unique_ptr<asio::io_service::work> _work_ptr;

	std::unordered_map<std::size_t, channel_context_ptr> _channel_contexts; // protected by _mutex
	channel_pipeline_ptr _channel_pipeline_ptr; // must be protected by _mutex before start() is called.
	std::vector<std::thread> _workers; // protected by _mutex

	std::mutex _mutex;
	std::condition_variable _cv;
};

} // namespace net
} // namespace shan

#include "tcp_service.h"
#include "udp_service.h"

#endif /* shan_net_service_h */
