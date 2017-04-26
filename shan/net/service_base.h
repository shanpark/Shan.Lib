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
	friend class tcp_channel_context_base;
	friend class udp_channel_context;
protected:
	static const int default_buffer_base_size = 4096;

	service_base(std::size_t worker_count, std::size_t buffer_base_size = default_buffer_base_size)
	: _worker_count(worker_count), _buffer_base_size(buffer_base_size)
	, _stat(CREATED), _work(_io_service), _channel_pipeline_ptr(std::make_shared<channel_pipeline<Protocol>>()) {}

	virtual ~service_base() {
		if (_stop_thread_ptr && _stop_thread_ptr->joinable())
			_stop_thread_ptr->join();
	}
	
public:
	void add_channel_handler(channel_handler<Protocol>* ch_handler_p) {
		add_channel_handler(channel_handler_ptr<Protocol>(ch_handler_p));
	}
	
	void add_channel_handler(channel_handler_ptr<Protocol> ch_handler_ptr) {
		std::lock_guard<std::mutex> lock(_shared_mutex);
		if (_stat == CREATED)
			_channel_pipeline_ptr->add_handler_ptr(ch_handler_ptr);
		else
			throw service_error("the service is already started");
	}

	typename channel_pipeline<Protocol>::handler_ptr get_channel_handler_ptr(std::size_t index) {
		return _channel_pipeline_ptr->get_handler_ptr(index);
	}

	/**
	 wait_stop() must not be called in any handler. A deadlock will be caused.
	 */
	void wait_stop() {
		std::unique_lock<std::mutex> lock(_service_mutex);
		if ((RUNNING <= _stat) && (_stat < STOPPED))
			_service_cv.wait(lock, [this](){ return (_stat == STOPPED); });
	}

	/**
	 requests the service to stop all tasks. and can be called in handlers.
	 but if you call this in two handlers concurrently, a deadlock can be caused.
	 */
	void request_stop() {
		std::lock_guard<std::mutex> lock(_service_mutex);
		if ((_stat == RUNNING) && !_stop_thread_ptr) {
			_stat = REQ_STOP;
			_stop_thread_ptr = std::make_shared<std::thread>([this](){
				std::lock_guard<std::mutex> lock(_service_mutex);
				stop();
			});
		}
	}

	void write_channel(std::size_t channel_id, object_ptr data_ptr) {
		if (_stat == RUNNING) {
			try {
				typename decltype(_channel_contexts)::mapped_type ch_ctx_ptr;
				{
					std::lock_guard<std::mutex> lock(_shared_mutex);
					ch_ctx_ptr = _channel_contexts.at(channel_id);
				}
				ch_ctx_ptr->fire_channel_write(data_ptr);
			} catch (const std::exception& e) {
				throw service_error(e.what());
			}
		}
		else {
			throw service_error("the service is not running.");
		}
	}

	void close_channel(std::size_t channel_id) noexcept {
		try {
			typename decltype(_channel_contexts)::mapped_type ch_ctx_ptr;
			{
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ch_ctx_ptr = _channel_contexts.at(channel_id);
			}
			ch_ctx_ptr->fire_channel_close();
		} catch (const std::exception&) {
			// if no such channel exists.
			// the channel will be closed. nothing happens.
		}
	}

	void fire_user_event_channel(std::size_t channel_id, int64_t id, object_ptr param_ptr) noexcept {
		try {
			typename decltype(_channel_contexts)::mapped_type ch_ctx_ptr;
			{
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ch_ctx_ptr = _channel_contexts.at(channel_id);
			}
			ch_ctx_ptr->fire_user_event(id, param_ptr);
		} catch (const std::exception&) {
			// if no such channel exists.
			// the channel was deleted
		}
	}

	channel_context<Protocol>* channel_context_of(std::size_t channel_id) noexcept {
		try {
			std::lock_guard<std::mutex> lock(_shared_mutex);
			return _channel_contexts.at(channel_id).get();
		} catch (const std::exception&) {
			// no such channel id
			return nullptr;
		}
	}

protected:
	void start() {
		_stat = RUNNING;

		// start worker threads
		for (std::size_t inx = 0 ; inx < _worker_count ; inx++)
			_workers.push_back(std::thread(std::bind<std::size_t(asio::io_service::*)()>(&asio::io_service::run, &_io_service)));
	}

	// Once stop() is called, the service object can not be reused.
	virtual void stop() {
		_stat = STOPPED;
		
		_io_service.stop();
		
		_join_worker_threads();
		
		// class all channels
		_close_all_channel_immediately();
	}

	const handler_vector<Protocol>& channel_handlers() const {
		return _channel_pipeline_ptr->handlers();
	}

	void read_complete(const asio::error_code& error, std::size_t bytes_transferred, channel_context_ptr<Protocol> ch_ctx_ptr) {
		ch_ctx_ptr->clear_task_in_progress(T_READ);

		if (error) {
			if ((asio::error::eof == error) ||
				(asio::error::operation_aborted == error) ||
				(asio::error::connection_reset == error) ||
				(asio::error::shut_down == error)) {
				// not treat as an error.
			}
			else {
				ch_ctx_ptr->fire_exception_caught(channel_error(error.message()));
			}

			std::size_t ch_id = ch_ctx_ptr->channel_id();
			ch_ctx_ptr->call_channel_disconnected(); // All errors cause end of the connection.

			// erase context
			{
				std::lock_guard<std::mutex> lock(_shared_mutex);
				_channel_contexts.erase(ch_id); // if ch_id not exists, nothing happens.
			}
		}
		else {
			ch_ctx_ptr->call_channel_read(bytes_transferred, ch_ctx_ptr->strand().wrap(std::bind(&service_base::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
		}

		// channel_read() 핸들러에서 close()를 호출했다면 context 상태는 REQ_CLOSE로 바뀌었을 것이고
		// close 작업을 수행할 lambda가 post되었을 것이다. 하지만 여기도 strand에서 실행되는 상태이므로
		// 만약 busy가 아니라면 여기서 바로 close_gracefully()를 호출한다.
		// 나중에 lambda가 호출되었을 때 다시 stat을 검사하므로 두 번 close를 수행하진 않는다.
		// busy 상태가 아니므로 cancel_all()을 호출하진 않는다.
		// 이 method는 clear_task_in_progress()가 호출되는 handler의 끝에서 매번 호출되어야
		// busy 상태가 해제되는 즉시 close_gracefully()가 호출 될 수 있다.
		ch_ctx_ptr->handle_req_close(ch_ctx_ptr->strand().wrap(std::bind(&service_base::shutdown_complete, this, std::placeholders::_1, ch_ctx_ptr)));
	}

	void write_complete(const asio::error_code& error, std::size_t bytes_transferred, channel_context_ptr<Protocol> ch_ctx_ptr) {
		// bytes_transferred: If an error occurred, this will be less than the sum of the buffer sizes. (not transferred or some transferred)
		ch_ctx_ptr->clear_task_in_progress(T_WRITE);
		ch_ctx_ptr->remove_sent_data();

		if (error) {
			ch_ctx_ptr->fire_exception_caught(channel_error(error.message()));
		}
		else {
			ch_ctx_ptr->call_channel_written(bytes_transferred);
		}

		ch_ctx_ptr->handle_req_close(ch_ctx_ptr->strand().wrap(std::bind(&service_base::shutdown_complete, this, std::placeholders::_1, ch_ctx_ptr)));
	}

	void shutdown_complete(const asio::error_code& error, channel_context_ptr<Protocol> ch_ctx_ptr) {
		ch_ctx_ptr->clear_task_in_progress(T_SHUTDOWN);
		ch_ctx_ptr->close_without_shutdown(); // shutdown completed. just close without shutdown.

		if (ch_ctx_ptr->connected()) {
			std::size_t ch_id = ch_ctx_ptr->channel_id();
			ch_ctx_ptr->call_channel_disconnected();

			// erase context
			{
				std::lock_guard<std::mutex> lock(_shared_mutex);
				_channel_contexts.erase(ch_id); // if ch_id not exists, nothing happens.
			}
		}
	}

	void _join_worker_threads() {
		for (auto& t : _workers) {
			if (t.joinable()) {
				try {
					t.join();
				} catch (const std::exception& e) {
					service_error(e.what());
				}
			}
		}
		_workers.clear();
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
		REQ_STOP,
		STOPPED
	};

protected:
	const std::size_t _worker_count;
	const std::size_t _buffer_base_size;

	uint8_t _stat;

	asio::io_service _io_service;
	asio::io_service::work _work;

	std::unordered_map<std::size_t, channel_context_ptr<Protocol>> _channel_contexts; // protected by _mutex
	channel_pipeline_ptr<Protocol> _channel_pipeline_ptr; // must be protected by _mutex before start() is called.
	std::vector<std::thread> _workers; // protected by _mutex

	std::mutex _shared_mutex; // protection for shared member.
			
	std::mutex _service_mutex; 			 // protection for service running state.
	std::condition_variable _service_cv; //
	std::shared_ptr<std::thread> _stop_thread_ptr; // thread
};

} // namespace net
} // namespace shan

#ifdef SHAN_NET_SSL_ENABLE
#include "ssl_service_base.h"
#endif

#endif /* shan_net_service_base_h */
