//
//  udp_service.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 15..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_udp_service_h
#define shan_net_udp_service_h

namespace shan {
namespace net {

class udp_service : public service<protocol::udp> {
public:
	udp_service(std::size_t worker_count = 2, std::size_t buffer_base_size = default_buffer_base_size)
	: service<protocol::udp>(worker_count, buffer_base_size) {}

	virtual ~udp_service() {
		stop();
		_join_worker_threads(false);
	}

	virtual void start() noexcept {
		std::lock_guard<std::mutex> lock(_service_mutex);
		if (!is_running()) {
			service::start();
			
			_resolver_ptr = std::unique_ptr<asio::ip::udp::resolver>(new asio::ip::udp::resolver(*_io_service_ptr));
		}
	}

	virtual void stop() noexcept {
		std::lock_guard<std::mutex> lock(_service_mutex);
		if (is_running()) {
			service::stop(); // no more handler will be called.
			
			_resolver_ptr = nullptr; // release ownership
			_service_cv.notify_all();
		}
	}

	bool resolve(const std::string& domain, uint16_t port, ip_port& ip_port_out) {
		if (is_running()) {
			asio::error_code error;
			asio::ip::udp::resolver::query query(domain, std::to_string(port));
			auto it = _resolver_ptr->resolve(query, error);

			ip_port_out.set(it->endpoint().address(), it->endpoint().port());
			return true;
		}
		return false;
	}

	void bind_connect(uint16_t local_port, const std::string& remote_address = std::string(""), uint16_t remote_port = 0, ip v = ip::v4) {
		if (is_running()) {
			udp_channel_context_ptr ch_ctx_ptr = std::make_shared<udp_channel_context>(udp_channel_ptr(new udp_channel(asio::ip::udp::socket(*_io_service_ptr), _buffer_base_size)), this);

			ch_ctx_ptr->open(v); // opened channel can only have an id.

			auto pair = std::make_pair(ch_ctx_ptr->channel_id(), ch_ctx_ptr);
			std::pair<decltype(_channel_contexts)::iterator, bool> ret;
			try {
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ret = _channel_contexts.emplace(pair);
			} catch (...) {
				ret.second = false;
			}

			if (!ret.second) { // not inserted. already exist. this is a critical error!
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("critical error. duplicate id for new channel."));
				ch_ctx_ptr->close_immediately();
			}
			else { // successfully inserted.
				ch_ctx_ptr->bind(local_port);
				fire_channel_bound(ch_ctx_ptr);
				if ((remote_address.length() == 0) || (remote_port == 0)) {
					ch_ctx_ptr->read(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
				}
				else {
					asio::ip::udp::resolver::query query(remote_address, std::to_string(remote_port));
					_resolver_ptr->async_resolve(query, std::bind(&udp_service::resolve_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
				}
			}
		}
	}

	void bind_connect(uint16_t local_port, ip_port destination, ip v = ip::v4) {
		if (is_running()) {
			udp_channel_context_ptr ch_ctx_ptr = std::make_shared<udp_channel_context>(udp_channel_ptr(new udp_channel(asio::ip::udp::socket(*_io_service_ptr), _buffer_base_size)), this);

			ch_ctx_ptr->open(v); // opened channel can only have an id.

			auto pair = std::make_pair(ch_ctx_ptr->channel_id(), ch_ctx_ptr);
			std::pair<decltype(_channel_contexts)::iterator, bool> ret;
			try {
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ret = _channel_contexts.emplace(pair);
			} catch (...) {
				ret.second = false;
			}

			if (!ret.second) { // not inserted. already exist. this is a critical error!
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("critical error. duplicate id for new channel."));
				ch_ctx_ptr->close_immediately();
			}
			else { // successfully inserted.
				ch_ctx_ptr->bind(local_port);
				fire_channel_bound(ch_ctx_ptr);
				if (!destination.is_valid()) {
					ch_ctx_ptr->read(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
				}
				else {
					ch_ctx_ptr->connect(destination.ip(), destination.port(), std::bind((&udp_service::connect_complete), this, std::placeholders::_1, ch_ctx_ptr));
				}
			}
		}
	}

	bool write_channel_to(std::size_t channel_id, const ip_port& destination, object_ptr data) {
		try {
			udp_channel_context_ptr ch_ctx_ptr;
			{
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ch_ctx_ptr = std::static_pointer_cast<udp_channel_context>(_channel_contexts.at(channel_id));
			}
			fire_channel_write_to(ch_ctx_ptr, data, destination);
			return true;
		} catch (const std::exception&) {
			return false;
		}
	}

private:
	virtual bool is_running() { return static_cast<bool>(_resolver_ptr); }

	void resolve_complete(const asio::error_code& error, asio::ip::udp::resolver::iterator it, udp_channel_context_ptr ch_ctx_ptr) {
		if (error)
			fire_channel_exception_caught(ch_ctx_ptr, resolver_error("fail to resolve address"));
		else
			ch_ctx_ptr->connect(it->endpoint().address().to_string(), it->endpoint().port(), std::bind((&udp_service::connect_complete), this, std::placeholders::_1, ch_ctx_ptr)); // try first endpoint unconditionally.
	}

	void connect_complete(const asio::error_code& error, udp_channel_context_ptr ch_ctx_ptr) {
		if (error)
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(error.message()));
		else
			fire_channel_connected(ch_ctx_ptr, std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
	}

	void fire_channel_bound(udp_channel_context_ptr ch_ctx_ptr) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr]() {
			if (!ch_ctx_ptr->set_stat_if_possible(channel_context<protocol::udp>::BOUND))
				return;

			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_bound(ch_ctx_ptr.get());
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_connected handler. (") + e.what() + ")"));
			}
		});
	}

	void fire_channel_connected(udp_channel_context_ptr ch_ctx_ptr, std::function<read_complete_handler> read_handler) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, read_handler]() {
			if (!ch_ctx_ptr->set_stat_if_possible(channel_context<protocol::udp>::CONNECTED))
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

	virtual void fire_channel_read(channel_context_ptr<protocol::udp> ch_ctx_ptr, util::streambuf_ptr& sb_ptr) {
		auto ep = static_cast<udp_channel_context*>(ch_ctx_ptr.get())->sender();
		ip_port ip(ep.address(), ep.port());

		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, sb_ptr, ip]() { //
			ch_ctx_ptr->done(false); // reset context to 'not done'.

			auto object_ptr = std::static_pointer_cast<object>(sb_ptr);
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_read_from(static_cast<udp_channel_context*>(ch_ctx_ptr.get()), object_ptr, ip);
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_read_from handler. (") + e.what() + ")"));
			}

			streambuf_pool::return_object(sb_ptr); // return for reuse.
		});
	}

	virtual void fire_channel_write(channel_context_ptr<protocol::udp> ch_ctx_ptr, object_ptr data) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, data]() {
			ch_ctx_ptr->done(false); // reset context to 'not done'.

			auto write_obj = data;
			// outbound -->
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_write(static_cast<udp_channel_context*>(ch_ctx_ptr.get()), write_obj);
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_write handler. (") + e.what() + ")"));
			}

			util::streambuf_ptr sb_ptr = std::dynamic_pointer_cast<util::streambuf>(write_obj);
			if (sb_ptr) {
				bool success = ch_ctx_ptr->write_streambuf(sb_ptr, std::bind(&udp_service::write_complete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ch_ctx_ptr));
				if (!success)
					fire_channel_exception_caught(ch_ctx_ptr, channel_error("The status of the channel is not suitable for sending data."));
			}
			else {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("The object to be transferred was not serialized to streambuf."));
			}
		});
	}

	void fire_channel_write_to(udp_channel_context_ptr ch_ctx_ptr, object_ptr data, const ip_port& destination) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, data, destination]() {
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
			if (sb_ptr) {
				bool success = static_cast<udp_channel_context*>(ch_ctx_ptr.get())->write_streambuf_to(sb_ptr, destination, std::bind(&udp_service::write_complete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ch_ctx_ptr));
				if (!success) {
					fire_channel_exception_caught(ch_ctx_ptr, channel_error("The status of the channel is not suitable for sending data."));
				}
			}
			else {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("The object to be transferred was not serialized to streambuf."));
			}
		});
	}

	virtual void fire_channel_written(channel_context_ptr<protocol::udp> ch_ctx_ptr, std::size_t bytes_transferred, util::streambuf_ptr sb_ptr) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, bytes_transferred, sb_ptr](){
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_written(static_cast<udp_channel_context*>(ch_ctx_ptr.get()), bytes_transferred, sb_ptr);
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_written handler. (") + e.what() + ")"));
			}
		});
	}

	virtual void fire_channel_disconnected(channel_context_ptr<protocol::udp> ch_ctx_ptr) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr](){
			// in case you called close() yourself, the state is already disconnected,
			// and fire_channel_disconnected() is already called so it should't be called again.
			if (ch_ctx_ptr->stat() == channel_context<protocol::udp>::CONNECTED) {
				ch_ctx_ptr->set_stat_if_possible(channel_context<protocol::udp>::DISCONNECTED);

				ch_ctx_ptr->done(false); // reset context to 'not done'.
				// <-- inbound
				auto begin = channel_handlers().begin();
				auto end = channel_handlers().end();
				try {
					for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
						(*it)->channel_disconnected(static_cast<udp_channel_context*>(ch_ctx_ptr.get()));
				} catch (const std::exception& e) {
					fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_disconnected handler. (") + e.what() + ")"));
				}
			}

			// close channel
			try {
				auto ch_id = ch_ctx_ptr->channel_id();
				ch_ctx_ptr->close_immediately();
				{
					std::lock_guard<std::mutex> lock(_shared_mutex);
					_channel_contexts.erase(ch_id);
				}
			} catch (const std::exception&) {
				// ignore all errors even if the above codes causes an error
			}
		});
	}

	virtual void fire_channel_close(channel_context_ptr<protocol::udp> ch_ctx_ptr) {
		fire_channel_disconnected(ch_ctx_ptr);
	}

private:
	std::unique_ptr<asio::ip::udp::resolver> _resolver_ptr;
};

} // namespace net
} // namespace shan

#endif /* shan_net_udp_service_h */
