//
//  tcp_service_base.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_service_base_h
#define shan_net_tcp_service_base_h

namespace shan {
namespace net {

class tcp_service_base : public service_base<protocol::tcp> {
public:
	tcp_service_base(std::size_t worker_count = 2, std::size_t buffer_base_size = default_buffer_base_size)
	: service_base<protocol::tcp>(worker_count, buffer_base_size) {}

protected:
	void call_channel_connected(tcp_channel_context_base_ptr ch_ctx_ptr, std::function<read_complete_handler> read_handler) {
		if (!ch_ctx_ptr->settable_stat(context_stat::CONNECTED))
			return;

		ch_ctx_ptr->stat(context_stat::CONNECTED);
		ch_ctx_ptr->connected(true);

		ch_ctx_ptr->done(false); // reset context to 'not done'.
		// <-- inbound
		auto begin = channel_handlers().begin();
		auto end = channel_handlers().end();
		try {
			for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
				(*it)->channel_connected(static_cast<tcp_channel_context_base*>(ch_ctx_ptr.get()));
		} catch (const std::exception& e) {
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_connected handler. (") + e.what() + ")"));
		}

		if (ch_ctx_ptr->stat() == CONNECTED) {
			ch_ctx_ptr->set_task_in_progress(T_READ);
			ch_ctx_ptr->read(read_handler);
		}

		ch_ctx_ptr->handle_req_close(ch_ctx_ptr->handler_strand().wrap(std::bind(&tcp_service_base::shutdown_complete, this, std::placeholders::_1, ch_ctx_ptr)));
	}

	virtual void call_channel_read(tcp_channel_context_base_ptr ch_ctx_ptr, std::size_t bytes_transferred, std::function<read_complete_handler> read_handler) override {
//...	ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, bytes_transferred, read_handler]() { //
			util::streambuf_ptr sb_ptr = ch_ctx_ptr->read_buf();
			sb_ptr->commit(bytes_transferred);

			// channel_read() handler
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			do {
				auto before_size = ch_ctx_ptr->read_buf()->in_size();

				auto object_ptr = std::static_pointer_cast<object>(ch_ctx_ptr->read_buf());
				// <-- inbound
				auto begin = channel_handlers().begin();
				auto end = channel_handlers().end();
				try {
					for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
						(*it)->channel_read(static_cast<tcp_channel_context_base*>(ch_ctx_ptr.get()), object_ptr);
				} catch (const std::exception& e) {
					fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_read handler. (") + e.what() + ")"));
				}

				if (ch_ctx_ptr->read_buf()->in_size() == before_size) // no data used.
					break;
			} while (ch_ctx_ptr->read_buf()->in_size() > 0); // available data remains.

			// channel_rdbuf_empty() handler
			if (ch_ctx_ptr->read_buf()->in_size() == 0) {
				if (ch_ctx_ptr->stat() == context_stat::CONNECTED) { // if channel_disconnected() is already called, don't call channel_rdbuf_empty().
					ch_ctx_ptr->done(false); // reset context to 'not done'.
					// <-- inbound
					auto begin = channel_handlers().begin();
					auto end = channel_handlers().end();
					try {
						for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
							(*it)->channel_rdbuf_empty(static_cast<tcp_channel_context_base*>(ch_ctx_ptr.get()));
					} catch (const std::exception& e) {
						fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_rdbuf_empty handler. (") + e.what() + ")"));
					}
				}
			}

			if (ch_ctx_ptr->stat() == CONNECTED) {
				ch_ctx_ptr->set_task_in_progress(T_READ);
				ch_ctx_ptr->read(read_handler);
			}
//		});
	}

	virtual void fire_channel_write(tcp_channel_context_base_ptr ch_ctx_ptr, object_ptr data) override {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, data]() {
			ch_ctx_ptr->done(false); // reset context to 'not done'.

			auto write_obj = data;
			// outbound -->
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_write(static_cast<tcp_channel_context_base*>(ch_ctx_ptr.get()), write_obj);
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_write handler. (") + e.what() + ")"));
			}

			util::streambuf_ptr sb_ptr = std::dynamic_pointer_cast<util::streambuf>(write_obj);
			if (sb_ptr) {
				if (ch_ctx_ptr->stat() == context_stat::CONNECTED) {
					ch_ctx_ptr->set_task_in_progress(T_WRITE);
					ch_ctx_ptr->write_streambuf(sb_ptr, ch_ctx_ptr->handler_strand().wrap(std::bind(&tcp_service_base::write_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
				}
				else {
					fire_channel_exception_caught(ch_ctx_ptr, channel_error("The status of the channel is not suitable for sending data."));
				}
			}
			else {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("The object to be transferred was not serialized to streambuf."));
			}
		});
	}

	virtual void call_channel_written(tcp_channel_context_base_ptr ch_ctx_ptr, std::size_t bytes_transferred) override {
		ch_ctx_ptr->done(false); // reset context to 'not done'.
		// <-- inbound
		auto begin = channel_handlers().begin();
		auto end = channel_handlers().end();
		try {
			for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
				(*it)->channel_written(static_cast<tcp_channel_context_base*>(ch_ctx_ptr.get()), bytes_transferred);
		} catch (const std::exception& e) {
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_written handler. (") + e.what() + ")"));
		}

		if (ch_ctx_ptr->stat() == context_stat::CONNECTED) {
			if (ch_ctx_ptr->has_data_to_write()) {
				ch_ctx_ptr->set_task_in_progress(T_WRITE);
				ch_ctx_ptr->write_next_streambuf(ch_ctx_ptr->handler_strand().wrap(std::bind(&tcp_service_base::write_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
			}
		}
	}

	virtual void call_channel_disconnected(tcp_channel_context_base_ptr ch_ctx_ptr) override {
		// call channel_disconnected() handler, if needed
		if (ch_ctx_ptr->connected()) {
			ch_ctx_ptr->connected(false);

			if (ch_ctx_ptr->settable_stat(context_stat::DISCONNECTED))
				ch_ctx_ptr->stat(context_stat::DISCONNECTED);

			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_disconnected(static_cast<tcp_channel_context_base*>(ch_ctx_ptr.get()));
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_disconnected handler. (") + e.what() + ")"));
			}
		}

		// close channel, if needed
		std::size_t ch_id = ch_ctx_ptr->channel_id();
		if (ch_ctx_ptr->settable_stat(context_stat::CLOSED))
			ch_ctx_ptr->close_immediately();

		// erase context
		{
			std::lock_guard<std::mutex> lock(_shared_mutex);
			_channel_contexts.erase(ch_id);
		}
	}

	virtual void fire_channel_close(tcp_channel_context_base_ptr ch_ctx_ptr) override {
		if ((ch_ctx_ptr->stat() >= context_stat::OPEN) && (ch_ctx_ptr->stat() < context_stat::REQ_CLOSE)) {
			ch_ctx_ptr->stat(context_stat::REQ_CLOSE); // must change stat to REQ_CLOSE instantly to prevent different operations from being fired.

			ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr](){
				if (ch_ctx_ptr->stat() == context_stat::REQ_CLOSE) {
					if (ch_ctx_ptr->is_channel_busy())
						ch_ctx_ptr->cancel_all(); // some complete handlers(read_complete, write_complete, ...) will be called.
					else
						ch_ctx_ptr->close_gracefully(ch_ctx_ptr->handler_strand().wrap(std::bind(&tcp_service_base::shutdown_complete, this, std::placeholders::_1, ch_ctx_ptr)));
				}
			});
		}
	}
};

} // namespace net
} // namespace shan

#include "tcp_server_base.h"
#include "tcp_server.h"

#include "tcp_client_base.h"
#include "tcp_client.h"

#ifdef SHAN_NET_SSL_ENABLE
#include "ssl_service_base.h"
#endif

#endif /* shan_net_tcp_service_base_h */
