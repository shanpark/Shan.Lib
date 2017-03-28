//
//  tcp_service.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_service_h
#define shan_net_tcp_service_h

namespace shan {
namespace net {

class tcp_service : public service {
public:
	tcp_service(std::size_t worker_count = 2, std::size_t buffer_base_size = default_buffer_base_size)
	: service(protocol::tcp, worker_count) {
	}

	bool write_to(std::size_t channel_id, object_ptr data) noexcept {
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

		ch_ctx_ptr->read(std::bind(&tcp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
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

	void fire_channel_connected(channel_context_ptr ch_ctx_ptr) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr]() {
			if (!ch_ctx_ptr->set_stat_if_possible(channel_context::connected))
				return;

			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().rbegin();
			auto end = channel_handlers().rend();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_connected(ch_ctx_ptr.get());
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_connected handler. (") + e.what() + ")"));
			}
			ch_ctx_ptr->read(std::bind(&tcp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
		});
	}

	void fire_channel_read(channel_context_ptr ch_ctx_ptr, util::streambuf_ptr& sb_ptr) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr, sb_ptr]() { //
			// copy to context's buffer
			ch_ctx_ptr->read_buf()->append(*sb_ptr);
			streambuf_pool::return_object(sb_ptr); // return for reuse.

			ch_ctx_ptr->done(false); // reset context to 'not done'.
			do {
				auto before_size = ch_ctx_ptr->read_buf()->in_size();

				auto object_ptr = std::static_pointer_cast<object>(ch_ctx_ptr->read_buf());
				// <-- inbound
				auto begin = channel_handlers().rbegin();
				auto end = channel_handlers().rend();
				try {
					for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
						(*it)->channel_read(ch_ctx_ptr.get(), object_ptr);
				} catch (const std::exception& e) {
					fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_read handler. (") + e.what() + ")"));
				}

				if (ch_ctx_ptr->stat() != channel_context::connected) // if close() called in handler...
					return;

				if (ch_ctx_ptr->read_buf()->in_size() == before_size) // no data used.
					break;
			} while (ch_ctx_ptr->read_buf()->in_size() > 0); // available data remains.

			if (ch_ctx_ptr->read_buf()->in_size() == 0)
				fire_channel_read_complete(ch_ctx_ptr);
		});
	}

	void fire_channel_read_complete(channel_context_ptr ch_ctx_ptr) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr]() {
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().rbegin();
			auto end = channel_handlers().rend();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_rdbuf_empty(ch_ctx_ptr.get());
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_rdbuf_empty handler. (") + e.what() + ")"));
			}
		});
	}

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
				ch_ctx_ptr->write_stream(sb_ptr, std::bind(&tcp_service::write_complete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ch_ctx_ptr));
			else
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("The object to be transferred was not serialized to streambuf."));
		});
	}

	void fire_channel_written(channel_context_ptr ch_ctx_ptr, std::size_t bytes_transferred, util::streambuf_ptr sb_ptr) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr, bytes_transferred, sb_ptr](){
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().rbegin();
			auto end = channel_handlers().rend();
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
			if (!ch_ctx_ptr->set_stat_if_possible(channel_context::disconnected)) // already disconnected...
				return; // return immediately

			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().rbegin();
			auto end = channel_handlers().rend();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_disconnected(ch_ctx_ptr.get());
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_disconnected handler. (") + e.what() + ")"));
			}

			try {
				auto ch_id = ch_ctx_ptr->channel_id();
				ch_ctx_ptr->channel_close();
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

	using service::fire_channel_exception_caught;
	void fire_channel_exception_caught(channel_context_ptr ch_ctx_ptr, const resolver_error& e) {
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
};

} // namespace net
} // namespace shan

#include "server.h"
#include "client.h"

#endif /* shan_net_tcp_service_h */
