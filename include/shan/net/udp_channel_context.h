//
//  udp_channel_context.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 29..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_udp_channel_context_h
#define shan_net_udp_channel_context_h

namespace shan {
namespace net {

class udp_channel_context : public udp_channel_context_base {
	friend class udp_service;
public:
	udp_channel_context(udp_channel_ptr ch_ptr, service_base<protocol::udp>* svc_p)
	: channel_context<protocol::udp>(ch_ptr->io_service(), svc_p), _channel_ptr(std::move(ch_ptr)) {}

	void write_to(const ip_port& destination, object_ptr data_ptr);

private:
	void bind(uint16_t port, ip v = ip::v4) {
		_channel_ptr->bind(port, v);
	}

	void write_streambuf_to(util::streambuf_ptr write_sb_ptr, const ip_port& destination, std::function<write_complete_handler> write_handler) {
		_channel_ptr->write_streambuf_to(write_sb_ptr, destination, write_handler);
	}

	asio::ip::udp::endpoint& sender() {
		return _channel_ptr->sender();
	}

	virtual channel_base<protocol::udp>* channel_p() override {
		return _channel_ptr.get();
	}

	virtual void close_gracefully(std::function<shutdown_complete_handler> shutdown_handler) noexcept override {
		stat(CLOSED);
		_channel_ptr->close_gracefully(shutdown_handler);
		shutdown_handler(asio::error_code());
	}

	void connect(asio::ip::udp::resolver::iterator it, std::function<udp_connect_complete_handler> connect_handler) {
		// be called in resolve_complete.
		_channel_ptr->connect(it, connect_handler);
	}

	virtual void call_channel_read(std::size_t bytes_transferred, std::function<read_complete_handler> read_handler) override {
		auto ep = sender();
		ip_port ip(ep.address(), ep.port());

		util::streambuf_ptr sb_ptr = read_buf();
		sb_ptr->commit(bytes_transferred);

		done(false); // reset context to 'not done'.

		auto object_ptr = std::static_pointer_cast<object>(sb_ptr);
		// <-- inbound
		auto begin = _service_p->channel_handlers().begin();
		auto end = _service_p->channel_handlers().end();
		try {
			call_channel_read_from_handler(begin, end, object_ptr, ip);
			sb_ptr->reset(_service_p->_buffer_base_size); // unused data is discarded in UDP.
		} catch (const std::exception& e) {
			fire_exception_caught(channel_error(std::string("An exception has thrown in channel_read_from handler. (") + e.what() + ")"));
		}

		if ((stat() == BOUND) || (stat() == CONNECTED)) {
			set_task_in_progress(T_READ);
			read(read_handler);
		}
	}

	void call_channel_bound(bool call_read, std::function<read_complete_handler> read_handler) {
		if (!settable_stat(context_stat::BOUND))
			return;
		stat(context_stat::BOUND);

		done(false); // reset context to 'not done'.
		// <-- inbound
		auto begin = _service_p->channel_handlers().begin();
		auto end = _service_p->channel_handlers().end();
		try {
			for (auto it = begin ; !done() && (it != end) ; it++)
				(*it)->channel_bound(this);
		} catch (const std::exception& e) {
			fire_exception_caught(channel_error(std::string("An exception has thrown in channel_connected handler. (") + e.what() + ")"));
		}

		if (call_read) {
			if (stat() == BOUND) {
				set_task_in_progress(T_READ);
				read(read_handler);
			}
		}
	}

	void fire_channel_write_to(object_ptr data_ptr, const ip_port& destination) {
		std::shared_ptr<udp_channel_context> ch_ctx_ptr = std::static_pointer_cast<udp_channel_context>(shared_from_this());
		strand().post([this, ch_ctx_ptr, data_ptr, destination]() {
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
				if ((stat() == BOUND) || (stat() == CONNECTED)) {
					set_task_in_progress(T_WRITE);
					write_streambuf_to(sb_ptr, destination, ch_ctx_ptr->strand().wrap(std::bind(&service_base<protocol::udp>::write_complete, _service_p, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
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

	virtual void call_channel_created_handler(typename handler_vector<protocol::udp>::const_iterator begin, typename handler_vector<protocol::udp>::const_iterator end) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_created(this, channel_p());
	}

	virtual void call_channel_connected_handler(typename handler_vector<protocol::udp>::const_iterator begin, typename handler_vector<protocol::udp>::const_iterator end) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_connected(this);
	}

	void call_channel_read_from_handler(typename handler_vector<protocol::udp>::const_iterator begin, typename handler_vector<protocol::udp>::const_iterator end, std::shared_ptr<object> obj_ptr, ip_port ip) {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_read_from(this, obj_ptr, ip);
	}

	virtual void call_channel_write_handler(typename handler_vector<protocol::udp>::const_iterator begin, typename handler_vector<protocol::udp>::const_iterator end, object_ptr& write_obj_ptr) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_write(this, write_obj_ptr);
	}

	virtual void call_channel_written_handler(typename handler_vector<protocol::udp>::const_iterator begin, typename handler_vector<protocol::udp>::const_iterator end, std::size_t bytes_transferred) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_written(this, bytes_transferred);
	}

	virtual void call_channel_disconnected_handler(typename handler_vector<protocol::udp>::const_iterator begin, typename handler_vector<protocol::udp>::const_iterator end) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_disconnected(this);
	}

	virtual void call_user_event_handler(typename handler_vector<protocol::udp>::const_iterator begin, typename handler_vector<protocol::udp>::const_iterator end, int64_t id, object_ptr param_ptr) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->user_event(this, id, param_ptr);
	}

	virtual void call_exception_caught_handler(typename handler_vector<protocol::udp>::const_iterator begin, typename handler_vector<protocol::udp>::const_iterator end, const channel_error& e) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->exception_caught(this, e);
	}

	virtual void call_exception_caught_handler(typename handler_vector<protocol::udp>::const_iterator begin, typename handler_vector<protocol::udp>::const_iterator end, const resolver_error& e) override {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->exception_caught(this, e);
	}

private:
	udp_channel_ptr _channel_ptr;
};

using udp_channel_context_ptr = std::shared_ptr<udp_channel_context>;

} // namespace net
} // namespace shan

#include <shan/net/udp_service.h>

namespace shan {
namespace net {

inline void udp_channel_context::write_to(const ip_port& destination, object_ptr data_ptr) {
	static_cast<udp_service*>(_service_p)->write_channel_to(channel_id(), destination, data_ptr);
}

} // namespace net
} // namespace shan

#endif /* shan_net_udp_channel_context_h */
