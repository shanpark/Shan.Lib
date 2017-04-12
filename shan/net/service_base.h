//
//  service_base.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 15..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_service_base_h
#define shan_net_service_base_h

namespace shan {
namespace net {

template<typename Protocol>
class service_base : public object {
	friend class channel_context<Protocol>;
protected:
	static const int default_buffer_base_size = 4096;

	service_base(std::size_t worker_count, std::size_t buffer_base_size = default_buffer_base_size)
	: _worker_count(worker_count), _buffer_base_size(buffer_base_size)
    , _stat(CREATED), _channel_pipeline_ptr(new channel_pipeline<Protocol>()) {}

public:
	bool add_channel_handler(channel_handler<Protocol>* ch_handler_p) {
		return add_channel_handler(channel_handler_ptr<Protocol>(ch_handler_p));
	}
	
	bool add_channel_handler(channel_handler_ptr<Protocol> ch_handler_ptr) {
		std::lock_guard<std::mutex> lock(_shared_mutex);
		if (!is_running()) {
			_channel_pipeline_ptr->add_handler(std::move(ch_handler_ptr));
			return true;
		}
		
		return false; // if service started, can't add handler.
	}

	////////////////////////////////////////////////////////////////////////////
	// wait_stop() must not be called in any handler. A deadlock will be caused.
	void wait_stop() {
		std::unique_lock<std::mutex> lock(_service_mutex);
		if (is_running())
			_service_cv.wait(lock, [this](){ return !is_running(); });

		// if stop() is called in hander, a thread will be in _workers.
		if (!_workers.empty()) {
			_join_worker_threads(false);

			_work_ptr = nullptr;
			_io_service_ptr = nullptr;
		}
	}

	bool write_channel(std::size_t channel_id, object_ptr data) noexcept {
		try {
			typename decltype(_channel_contexts)::mapped_type ch_ctx_ptr;
			{
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ch_ctx_ptr = _channel_contexts.at(channel_id);
			}
			fire_channel_write(ch_ctx_ptr, data);
			return true;
		} catch (const std::exception& e) {
			std::cout << "write_channel():" << e.what() << "!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@@!@!@!@!@!@!@!@!@!@!!@!" << std::endl; //... 삭제 예정.
			return false;
		}
	}

	void close_channel(std::size_t channel_id) noexcept {
		try {
			typename decltype(_channel_contexts)::mapped_type ch_ctx_ptr;
			{
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ch_ctx_ptr = _channel_contexts.at(channel_id);
			}
			fire_channel_close(ch_ctx_ptr);
		} catch (const std::exception&) {
			// if no such channel exists, nothing happens.
			std::cout << "close_channel() exception !@!@!@!@!@!@!@!@!@!!@!@!@!@!@!@@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!" << std::endl; //... 삭제 예정.
		}
	}

protected:
	virtual bool is_running() {
		return (_stat == RUNNING);
	}

	void start() {
		_stat = RUNNING;
		_io_service_ptr = std::unique_ptr<asio::io_service>(new asio::io_service());
		_work_ptr = std::unique_ptr<asio::io_service::work>(new asio::io_service::work(*_io_service_ptr));

		// start worker threads
		for (std::size_t inx = 0 ; inx < _worker_count ; inx++)
			_workers.push_back(std::thread(std::bind<std::size_t(asio::io_service::*)()>(&asio::io_service::run, _io_service_ptr.get())));
	}

	// Once stop() is called, the service object can not be reused.
	void stop() {
		_stat = STOPPED;

		// class all channels
		_close_all_channel_immediately();

		_io_service_ptr->stop();

		_join_worker_threads(true);

		if (_workers.empty()) {
			_work_ptr = nullptr; // release ownership
			_io_service_ptr = nullptr; // release ownership.
		}
	}

	const std::vector<channel_handler_ptr<Protocol>>& channel_handlers() const {
		return _channel_pipeline_ptr->handlers();
	}

	void read_complete(const asio::error_code& error, util::streambuf_ptr sb_ptr, channel_context_ptr<Protocol> ch_ctx_ptr) {
		if (error) {
			streambuf_pool::return_object(sb_ptr); // sb_ptr will not be used. sb_ptr should be returned to the pool.

			if ((asio::error::eof == error) ||
				(asio::error::operation_aborted == error) ||
				(asio::error::connection_reset == error) ||
				(asio::error::shut_down == error)) {
				// not treat as an error.
			}
			else {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(error.message()));
			}

			fire_channel_disconnected(ch_ctx_ptr); // All errors cause end of the connection.
		}
		else {
			fire_channel_read(ch_ctx_ptr, sb_ptr, std::bind(&service_base::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
		}
	}

	void write_complete(const asio::error_code& error, std::size_t bytes_transferred, util::streambuf_ptr sb_ptr, channel_context_ptr<Protocol> ch_ctx_ptr) {
		// bytes_transferred: If an error occurred, this will be less than the sum of the buffer sizes. (not transferred or some transferred)
		if (error) {
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(error.message()));
		}
		else {
			fire_channel_written(ch_ctx_ptr, bytes_transferred, sb_ptr);
		}
	}

	void shutdown_complete(bool close_channel, channel_context_ptr<Protocol> ch_ctx_ptr) {
		if (close_channel) {
			ch_ctx_ptr->handler_strand().post([ch_ctx_ptr](){
				ch_ctx_ptr->close_without_shutdown(); // shutdown completed. just close without shutdown.

				if (ch_ctx_ptr->connected()) {
					std::cout << "---@@@>>> closed but connected" << std::endl;
				}

			});
		}
	}

	virtual void fire_channel_read(channel_context_ptr<Protocol> ch_ctx_ptr, util::streambuf_ptr& sb_ptr, std::function<read_complete_handler> read_handler) = 0;
	virtual void fire_channel_write(channel_context_ptr<Protocol> ch_ctx_ptr, object_ptr data) = 0;
	virtual void fire_channel_written(channel_context_ptr<Protocol> ch_ctx_ptr, std::size_t bytes_transferred, util::streambuf_ptr sb_ptr) = 0;
	virtual void fire_channel_disconnected(channel_context_ptr<Protocol> ch_ctx_ptr) = 0;
	virtual void fire_channel_close(channel_context_ptr<Protocol> ch_ctx_ptr) = 0;

	void fire_channel_exception_caught(channel_context_ptr<Protocol> ch_ctx_ptr, const channel_error& e) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, e]() {
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->exception_caught(ch_ctx_ptr.get(), e);
			} catch (const std::exception& e2) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in exception_caught handler. (") + e2.what() + ")")); // can cause infinite exception...
			}
		});
	}
	
	void fire_channel_exception_caught(channel_context_ptr<Protocol> ch_ctx_ptr, const resolver_error& e) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, e]() {
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->exception_caught(ch_ctx_ptr.get(), e);
			} catch (const std::exception& e2) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in exception_caught handler. (") + e2.what() + ")")); // can cause infinite exception...
			}
		});
	}

	void _join_worker_threads(bool skip_cur_thread) {
		std::thread cur_t;
		for (auto& t : _workers) {
			if (t.joinable()) {
				try {
					t.join();
				}
				catch (const std::exception& e) { // A deadlock exception occurs when calling stop() within an io_service run thread.
					if (skip_cur_thread && (t.get_id() == std::this_thread::get_id())) // current thread
						cur_t = std::move(t); // can not join current turead, so keep this thread object.
					else
						throw service_error(e.what());
				}
			}
		}
		_workers.clear();
		if (cur_t.joinable()) // push again current thread object.
			_workers.push_back(std::move(cur_t));
	}

	void _close_all_channel_immediately() {
		std::lock_guard<std::mutex> lock(_shared_mutex);
		for (auto pair : _channel_contexts)
			pair.second->close_immediately();
		_channel_contexts.clear();
	}

	enum : uint8_t {
		CREATED = 0,
		RUNNING,
		STOPPED
	};

protected:
	const std::size_t _worker_count;
	const std::size_t _buffer_base_size;

	uint8_t _stat;

	std::unique_ptr<asio::io_service> _io_service_ptr;
	std::unique_ptr<asio::io_service::work> _work_ptr;

	std::unordered_map<std::size_t, channel_context_ptr<Protocol>> _channel_contexts; // protected by _mutex
	channel_pipeline_ptr<Protocol> _channel_pipeline_ptr; // must be protected by _mutex before start() is called.
	std::vector<std::thread> _workers; // protected by _mutex

	std::mutex _shared_mutex; // protection for shared member.

	std::mutex _service_mutex; 			 // protection for service running state.
	std::condition_variable _service_cv; //
};

} // namespace net
} // namespace shan

#include "tcp_service_base.h"
#include "udp_service.h"

#endif /* shan_net_service_base_h */
