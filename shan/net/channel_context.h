//
//  channel_context.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 17..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_channel_context_h
#define shan_net_channel_context_h

namespace shan {
namespace net {

template<typename Protocol>
class service_base;


template<typename Protocol>
class channel_context : public context_base {
	friend class service_base<Protocol>;
	friend class tcp_server_base;
	friend class tcp_client_base;
	friend class tcp_server;
	friend class tcp_client;
	friend class ssl_server;
	friend class ssl_client;
	friend class udp_service;
	friend class tcp_idle_monitor;

public:
	using ptr = std::shared_ptr<channel_context<Protocol>>;

public:
	channel_context(asio::io_service& io_service, service_base<Protocol>* svc_p);

	virtual ~channel_context() {
		std::cout << ">>>> channel_context destoryed" << std::endl;
		streambuf_pool::return_object(_read_sb_ptr);
	}

	std::size_t channel_id() {
		return channel_p()->id();
	}

	void write(object_ptr data_ptr);
	void close();

	void fire_user_event(int64_t id, object_ptr param_ptr) {
		strand().post([this, id, param_ptr](){
			done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = _service_p->channel_handlers().begin();
			auto end = _service_p->channel_handlers().end();
			try {
				call_user_event_handler(begin, end, id, param_ptr);
			} catch (const std::exception& e) {
				fire_exception_caught(channel_error(std::string("An exception has thrown in user_event handler. (") + e.what() + ")"));
			}
		});
	}

	service_base<Protocol>* service_p() {
		return _service_p;
	}

protected:
	virtual channel_base<Protocol>* channel_p() = 0;

	void open(ip v) {
		if (settable_stat(OPEN)) {
			channel_p()->open(v);
			stat(OPEN);
		}
	}

	virtual void close_gracefully(std::function<shutdown_complete_handler> shutdown_handler) noexcept = 0;

	void close_immediately() noexcept {
		stat(CLOSED);
		channel_p()->close_immediately();
	}

	void close_without_shutdown() noexcept {
		stat(CLOSED);
		channel_p()->close_without_shutdown();
	}

	void cancel_all() {
		channel_p()->cancel_all();
	}

	// try connect without resolving an address.
	void connect(const ip_port& destination, std::function<connect_complete_handler> connect_handler) {
		strand().post([this, destination, connect_handler](){
			channel_p()->connect(destination, connect_handler);
		});
	}

	void read(std::function<read_complete_handler> read_handler) noexcept {
		channel_p()->read(_read_sb_ptr, read_handler);
	}

	void write_streambuf(util::streambuf_ptr write_sb_ptr) {
		channel_p()->write_streambuf(write_sb_ptr, strand().wrap(std::bind(&service_base<Protocol>::write_complete, _service_p, std::placeholders::_1, std::placeholders::_2, std::static_pointer_cast<channel_context<Protocol>>(shared_from_this()))));
	}

	void remove_sent_data() {
		channel_p()->remove_sent_data();
	}

	bool has_data_to_write() {
		return channel_p()->has_data_to_write();
	}

	void write_next_streambuf() {
		channel_p()->write_next_streambuf(strand().wrap(std::bind(&service_base<Protocol>::write_complete, _service_p, std::placeholders::_1, std::placeholders::_2, std::static_pointer_cast<channel_context<Protocol>>(shared_from_this()))));
	}

	void handle_req_close(std::function<shutdown_complete_handler> shutdown_handler) {
		if (stat() == REQ_CLOSE) {
			if (!is_channel_busy())
				close_gracefully(shutdown_handler);
		}
	}

	util::streambuf_ptr read_buf() {
		return _read_sb_ptr;
	}

	asio::io_service::strand& strand() {
		return _handler_strand;
	}

	asio::io_service& io_service() {
		return _handler_strand.get_io_service();
	}

	void call_channel_created() {
		done(false); // reset context to 'not done'.
		// <-- inbound
		auto begin = _service_p->channel_handlers().begin();
		auto end = _service_p->channel_handlers().end();
		try {
			call_channel_created_handler(begin, end);
		} catch (const std::exception& e) {
			fire_exception_caught(channel_error(std::string("An exception has thrown in channel_created handler. (") + e.what() + ")"));
		}
	}

	void call_channel_connected(std::function<read_complete_handler> read_handler) {
		if (!settable_stat(context_stat::CONNECTED))
			return;

		stat(context_stat::CONNECTED);
		connected(true);

		done(false); // reset context to 'not done'.
		// <-- inbound
		auto begin = _service_p->channel_handlers().begin();
		auto end = _service_p->channel_handlers().end();
		try {
			call_channel_connected_handler(begin, end);
		} catch (const std::exception& e) {
			fire_exception_caught(channel_error(std::string("An exception has thrown in channel_connected handler. (") + e.what() + ")"));
		}

		if (stat() == CONNECTED) {
			set_task_in_progress(T_READ);
			read(read_handler);
		}

		handle_req_close(strand().wrap(std::bind(&service_base<Protocol>::shutdown_complete, _service_p, std::placeholders::_1, std::static_pointer_cast<channel_context<Protocol>>(shared_from_this()))));
	}

	virtual void call_channel_read(std::size_t bytes_transferred, std::function<read_complete_handler> read_handler) = 0;

	void fire_channel_write(object_ptr data_ptr) {
		std::shared_ptr<channel_context<Protocol>> ch_ctx_ptr = std::static_pointer_cast<channel_context<Protocol>>(shared_from_this());
		strand().post([this, ch_ctx_ptr, data_ptr]() {
			done(false); // reset context to 'not done'.

			auto write_obj_ptr = data_ptr;
			// outbound -->
			auto begin = _service_p->channel_handlers().begin();
			auto end = _service_p->channel_handlers().end();
			try {
				call_channel_write_handler(begin, end, write_obj_ptr);
			} catch (const std::exception& e) {
				fire_exception_caught(channel_error(std::string("An exception has thrown in channel_write handler. (") + e.what() + ")"));
			}

			util::streambuf_ptr sb_ptr = std::dynamic_pointer_cast<util::streambuf>(write_obj_ptr);
			if (sb_ptr) {
				if ((stat() == context_stat::BOUND) || (stat() == context_stat::CONNECTED)) {
					set_task_in_progress(T_WRITE);
					write_streambuf(sb_ptr);
				}
				else {
					fire_exception_caught(channel_error("The status of the channel is not suitable for sending data."));
				}
			}
			else {
				fire_exception_caught(channel_error("The object to be transferred was not serialized to streambuf."));
			}
		});
	}

	void call_channel_written(std::size_t bytes_transferred) {
		if ((stat() == context_stat::BOUND) || (stat() == context_stat::CONNECTED)) { // if channel_disconnected() is already called, don't call channel_written().
			done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = _service_p->channel_handlers().begin();
			auto end = _service_p->channel_handlers().end();
			try {
				call_channel_written_handler(begin, end, bytes_transferred);
			} catch (const std::exception& e) {
				fire_exception_caught(channel_error(std::string("An exception has thrown in channel_written handler. (") + e.what() + ")"));
			}
		}

		if ((stat() == context_stat::BOUND) || (stat() == context_stat::CONNECTED)) {
			if (has_data_to_write()) {
				set_task_in_progress(T_WRITE);
				write_next_streambuf();
			}
		}
	}

	void call_channel_disconnected() {
		// call channel_disconnected() handler, if needed
		if (connected()) {
			connected(false);

			if (settable_stat(context_stat::DISCONNECTED))
				stat(context_stat::DISCONNECTED);

			done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = _service_p->channel_handlers().begin();
			auto end = _service_p->channel_handlers().end();
			try {
				call_channel_disconnected_handler(begin, end);
			} catch (const std::exception& e) {
				fire_exception_caught(channel_error(std::string("An exception has thrown in channel_disconnected handler. (") + e.what() + ")"));
			}
		}

		// close channel, if needed
		if (settable_stat(context_stat::CLOSED))
			close_immediately();
	}

	void fire_channel_close() {
		if ((stat() >= context_stat::OPEN) && (stat() < context_stat::REQ_CLOSE)) {
			stat(context_stat::REQ_CLOSE); // must change stat to REQ_CLOSE instantly to prevent different operations from being fired.

			std::shared_ptr<channel_context<Protocol>> ch_ctx_ptr = std::static_pointer_cast<channel_context<Protocol>>(shared_from_this());
			strand().post([this, ch_ctx_ptr](){
				if (stat() == context_stat::REQ_CLOSE) {
					if (is_channel_busy())
						cancel_all(); // some complete handlers(read_complete, write_complete, ...) will be called.
					else
						close_gracefully(ch_ctx_ptr->strand().wrap(std::bind(&service_base<Protocol>::shutdown_complete, _service_p, std::placeholders::_1, ch_ctx_ptr)));
				}
			});
		}
	}

	void fire_exception_caught(const channel_error& e) {
		std::shared_ptr<channel_context<Protocol>> ch_ctx_ptr = std::static_pointer_cast<channel_context<Protocol>>(shared_from_this());
		strand().post([this, ch_ctx_ptr, e]() {
			done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = _service_p->channel_handlers().begin();
			auto end = _service_p->channel_handlers().end();
			try {
				call_exception_caught_handler(begin, end, e);
			} catch (const std::exception& e2) {
				fire_exception_caught(channel_error(std::string("An exception has thrown in exception_caught handler. (") + e2.what() + ")")); // can cause infinite exception...
			}
		});
	}

	void fire_exception_caught(const resolver_error& e) {
		std::shared_ptr<channel_context<Protocol>> ch_ctx_ptr = std::static_pointer_cast<channel_context<Protocol>>(shared_from_this());
		strand().post([this, ch_ctx_ptr, e]() {
			done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = _service_p->channel_handlers().begin();
			auto end = _service_p->channel_handlers().end();
			try {
				call_exception_caught_handler(begin, end, e);
			} catch (const std::exception& e2) {
				fire_exception_caught(channel_error(std::string("An exception has thrown in exception_caught handler. (") + e2.what() + ")")); // can cause infinite exception...
			}
		});
	}

	virtual void call_channel_created_handler(typename handler_vector<Protocol>::const_iterator begin, typename handler_vector<Protocol>::const_iterator end) = 0;
	virtual void call_channel_connected_handler(typename handler_vector<Protocol>::const_iterator begin, typename handler_vector<Protocol>::const_iterator end) = 0;
	virtual void call_channel_write_handler(typename handler_vector<Protocol>::const_iterator begin, typename handler_vector<Protocol>::const_iterator end, object_ptr& write_obj_ptr) = 0;
	virtual void call_channel_written_handler(typename handler_vector<Protocol>::const_iterator begin, typename handler_vector<Protocol>::const_iterator end, std::size_t bytes_transferred) = 0;
	virtual void call_channel_disconnected_handler(typename handler_vector<Protocol>::const_iterator begin, typename handler_vector<Protocol>::const_iterator end) = 0;

	virtual void call_user_event_handler(typename handler_vector<Protocol>::const_iterator begin, typename handler_vector<Protocol>::const_iterator end, int64_t id, object_ptr param_ptr) = 0;
	virtual void call_exception_caught_handler(typename handler_vector<Protocol>::const_iterator begin, typename handler_vector<Protocol>::const_iterator end, const channel_error& e) = 0;
	virtual void call_exception_caught_handler(typename handler_vector<Protocol>::const_iterator begin, typename handler_vector<Protocol>::const_iterator end, const resolver_error& e) = 0;

protected:
	service_base<Protocol>* _service_p;
	util::streambuf_ptr _read_sb_ptr;
	asio::io_service::strand _handler_strand;
};

template<typename Protocol>
using channel_context_ptr = typename channel_context<Protocol>::ptr;

using tcp_channel_context_base_ptr = std::shared_ptr<tcp_channel_context_base>;
using udp_channel_context_base_ptr = udp_channel_context_base::ptr;

} // namespace net
} // namespace shan

#include "service_base.h"

#include "tcp_channel_context_base.h"
#include "udp_channel_context.h"

namespace shan {
namespace net {

template<typename Protocol>
inline channel_context<Protocol>::channel_context(asio::io_service& io_service, service_base<Protocol>* svc_p)
: context_base(), _service_p(svc_p), _read_sb_ptr(streambuf_pool::get_object(svc_p->_buffer_base_size)), _handler_strand(io_service) {}

template<typename Protocol>
inline void channel_context<Protocol>::write(object_ptr data_ptr) {
	_service_p->write_channel(channel_id(), data_ptr);
}

template<typename Protocol>
inline void channel_context<Protocol>::close() {
	_service_p->close_channel(channel_id());
}

} // namespace net
} // namespace shan

#endif /* shan_net_channel_context_h */
