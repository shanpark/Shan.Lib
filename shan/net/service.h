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

	bool write_channel(std::size_t channel_id, object_ptr data) noexcept {
		try {
			channel_context_ptr ch_ctx_ptr;
			{
				std::lock_guard<std::mutex> lock(_mutex);
				ch_ctx_ptr = _channel_contexts.at(channel_id);
			}
			fire_channel_write(ch_ctx_ptr, data);
			return true;
		} catch (const std::exception& e) {
			return false;
		}
	}

	void close_channel(std::size_t channel_id) noexcept {
		try {
			channel_context_ptr ch_ctx_ptr;
			{
				std::lock_guard<std::mutex> lock(_mutex);
				ch_ctx_ptr = _channel_contexts.at(channel_id);
			}
			fire_channel_close(ch_ctx_ptr);
		} catch (const std::exception& e) {
			// if no such channel exists, nothing happens.
		}
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
			pair.second->close_immediately();
		_channel_contexts.clear();

		_work_ptr = nullptr; // release ownership
		_io_service_ptr = nullptr; // release ownership.
	}

	const std::vector<channel_pipeline::handler_ptr>& channel_handlers() const { return _channel_pipeline_ptr->handlers(); }

	void read_complete(const asio::error_code& error, util::streambuf_ptr sb_ptr, channel_context_ptr ch_ctx_ptr) {
		if (error) {
			streambuf_pool::return_object(sb_ptr); // sb_ptr will not be used. sb_ptr should be returned to the pool.

			if ((asio::error::eof == error) ||
				(asio::error::connection_reset == error) ||
				(asio::error::operation_aborted == error) ||
				(asio::error::shut_down == error)) {
				fire_channel_disconnected(ch_ctx_ptr);
				return;
			}
			else {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(error.message()));
			}
		}
		else {
			fire_channel_read(ch_ctx_ptr, sb_ptr);
		}

		ch_ctx_ptr->read(std::bind(&service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
	}

	void write_complete(const asio::error_code& error, std::size_t bytes_transferred, util::streambuf_ptr sb_ptr, channel_context_ptr ch_ctx_ptr) {
		// bytes_transferred: If an error occurred, this will be less than the sum of the buffer sizes. (not transferred or some transferred)
		if (error) {
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(error.message()));
		}
		else {
			fire_channel_written(ch_ctx_ptr, bytes_transferred, sb_ptr);
		}
	}
	
	void fire_channel_connected(channel_context_ptr ch_ctx_ptr, std::function<read_complete_handler> read_handler) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr, read_handler]() {
			if (!ch_ctx_ptr->set_stat_if_possible(channel_context::CONNECTED))
				return;

			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_connected(ch_ctx_ptr.get());
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_connected handler. (") + e.what() + ")"));
			}
			ch_ctx_ptr->read(read_handler);
		});
	}

	virtual void fire_channel_read(channel_context_ptr ch_ctx_ptr, util::streambuf_ptr& sb_ptr) = 0;

	void fire_channel_write(channel_context_ptr ch_ctx_ptr, object_ptr data) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr, data]() {
			ch_ctx_ptr->done(false); // reset context to 'not done'.

			auto write_obj = data;
			// outbound -->
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_write(ch_ctx_ptr.get(), write_obj);
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_write handler. (") + e.what() + ")"));
			}

			util::streambuf_ptr sb_ptr = std::dynamic_pointer_cast<util::streambuf>(write_obj);
			if (sb_ptr)
				ch_ctx_ptr->write_streambuf(sb_ptr, std::bind(&service::write_complete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ch_ctx_ptr));
			else
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("The object to be transferred was not serialized to streambuf."));
		});
	}

	void fire_channel_written(channel_context_ptr ch_ctx_ptr, std::size_t bytes_transferred, util::streambuf_ptr sb_ptr) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr, bytes_transferred, sb_ptr](){
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_written(ch_ctx_ptr.get(), bytes_transferred, sb_ptr);
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_written handler. (") + e.what() + ")"));
			}
		});
	}

	void fire_channel_disconnected(channel_context_ptr ch_ctx_ptr) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr](){
			// in case you called close() yourself, the state is already disconnected,
			// and fire_channel_disconnected() is already called so it should't be called again.
			if (ch_ctx_ptr->stat() == channel_context::CONNECTED) {
				ch_ctx_ptr->set_stat_if_possible(channel_context::DISCONNECTED);

				ch_ctx_ptr->done(false); // reset context to 'not done'.
				// <-- inbound
				auto begin = channel_handlers().begin();
				auto end = channel_handlers().end();
				try {
					for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
						(*it)->channel_disconnected(ch_ctx_ptr.get());
				} catch (const std::exception& e) {
					fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_disconnected handler. (") + e.what() + ")"));
				}
			}

			// close channel
			try {
				auto ch_id = ch_ctx_ptr->channel_id();
				ch_ctx_ptr->close_immediately();
				{
					std::lock_guard<std::mutex> lock(_mutex);
					_channel_contexts.erase(ch_id);
				}
			} catch (const std::exception& e) {
				// ignore all errors even if the above codes causes an error
			}
		});
	}

	void fire_channel_close(channel_context_ptr ch_ctx_ptr) {
		fire_channel_disconnected(ch_ctx_ptr);
	}

	void fire_channel_exception_caught(channel_context_ptr ch_ctx_ptr, const channel_error& e) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr, e]() {
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
	
	void fire_channel_exception_caught(channel_context_ptr ch_ctx_ptr, const resolver_error& e) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr, e]() {
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
