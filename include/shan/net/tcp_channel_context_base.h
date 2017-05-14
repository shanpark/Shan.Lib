//
//  tcp_channel_context_base.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 20..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_channel_context_base_h
#define shan_net_tcp_channel_context_base_h

namespace shan {
namespace net {

class tcp_channel_context_base : public channel_context<protocol::tcp> {
public:
	tcp_channel_context_base(asio::io_service& io_service, service_base<protocol::tcp>* svc_p)
	: channel_context<protocol::tcp>(io_service, svc_p) {}

protected:
	virtual void call_channel_read(std::size_t bytes_transferred, std::function<read_complete_handler> read_handler) override {
		util::streambuf_ptr sb_ptr = read_buf();
		sb_ptr->commit(bytes_transferred);

		// channel_read() handler
		done(false); // reset context to 'not done'.
		do {
			auto before_size = read_buf()->in_size();

			auto object_ptr = std::static_pointer_cast<object>(read_buf());
			// <-- inbound
			auto begin = _service_p->channel_handlers().begin();
			auto end = _service_p->channel_handlers().end();
			try {
				call_channel_read_handler(begin, end, object_ptr);
			} catch (const std::exception& e) {
				fire_exception_caught(channel_error(std::string("An exception has thrown in channel_read handler. (") + e.what() + ")"));
			}

			if (read_buf()->in_size() == before_size) // no data used.
				break;
		} while (read_buf()->in_size() > 0); // available data remains.

		// channel_rdbuf_empty() handler
		if (read_buf()->in_size() == 0) {
			if (stat() == context_stat::CONNECTED) { // if channel_disconnected() is already called, don't call channel_rdbuf_empty().
				done(false); // reset context to 'not done'.
				// <-- inbound
				auto begin = _service_p->channel_handlers().begin();
				auto end = _service_p->channel_handlers().end();
				try {
					call_channel_rdbuf_empty_handler(begin, end);
				} catch (const std::exception& e) {
					fire_exception_caught(channel_error(std::string("An exception has thrown in channel_rdbuf_empty handler. (") + e.what() + ")"));
				}
			}
		}

		if (stat() == CONNECTED) {
			set_task_in_progress(T_READ);
			read(read_handler);
		}
	}
	
	virtual void call_channel_created_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_created(this, static_cast<tcp_channel_base*>(channel_p()));
	}
	
	virtual void call_channel_connected_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_connected(this);
	}

	// non-virtual
	void call_channel_read_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end, std::shared_ptr<object> obj_ptr) {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_read(this, obj_ptr);
	}
	
	// non-virtual
	void call_channel_rdbuf_empty_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end) {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_rdbuf_empty(this);
	}

	virtual void call_channel_write_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end, object_ptr& write_obj_ptr) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_write(this, write_obj_ptr);
	}
	
	virtual void call_channel_written_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end, std::size_t bytes_transferred) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_written(this, bytes_transferred);
	}

	virtual void call_channel_disconnected_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_disconnected(this);
	}

	virtual void call_user_event_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end, int64_t id, object_ptr param_ptr) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->user_event(this, id, param_ptr);
	}

	virtual void call_exception_caught_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end, const channel_error& e) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->exception_caught(this, e);
	}

	virtual void call_exception_caught_handler(typename handler_vector<protocol::tcp>::const_iterator begin, typename handler_vector<protocol::tcp>::const_iterator end, const resolver_error& e) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->exception_caught(this, e);
	}
};

} // namespace net
} // namespace shan

#ifdef SHAN_NET_SSL_ENABLE
#include <shan/net/ssl_channel_context.h>
#endif
#include <shan/net/tcp_channel_context.h>

#endif /* shan_net_tcp_channel_context_base_h */
