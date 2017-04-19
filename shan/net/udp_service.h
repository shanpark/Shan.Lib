//
//  udp_service.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 15..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//
// Note: udp_channel's read(), write_streambuf() or write_streambuf_to() can
//  cause ECONNREFUSED error if no receiver was associated with the destination
//  address

#ifndef shan_net_udp_service_h
#define shan_net_udp_service_h

namespace shan {
namespace net {

class udp_service : public service_base<protocol::udp> {
public:
	udp_service(std::size_t worker_count = 2, std::size_t buffer_base_size = default_buffer_base_size)
	: service_base<protocol::udp>(worker_count, buffer_base_size), _resolver(_io_service) {}

	virtual ~udp_service() {
		// Note: service's destructor must not be called in worker thread(channel handler)
		std::lock_guard<std::mutex> lock(_service_mutex);
		if (_stat == RUNNING)
			stop();
	}

	void start() {
		std::lock_guard<std::mutex> lock(_service_mutex);
		if (_stat == CREATED) {
			service_base::start();
		}
	}

	void resolve(const std::string& domain, uint16_t port, ip_port& ip_port_out) {
		if (_stat == RUNNING) {
			asio::error_code error;
			asio::ip::udp::resolver::query query(domain, std::to_string(port));
			auto it = _resolver.resolve(query, error);

			ip_port_out.set(it->endpoint().address(), it->endpoint().port());
		}
		else {
			throw service_error("the service is not running.");
		}
	}

	void bind_connect(uint16_t local_port, const std::string& remote_address = std::string(""), uint16_t remote_port = 0, ip v = ip::v4) {
		if (_stat == RUNNING) {
			udp_channel_context_ptr ch_ctx_ptr = std::make_shared<udp_channel_context>(std::make_shared<udp_channel>(asio::ip::udp::socket(_io_service), _buffer_base_size), this);

			auto pair = std::make_pair(ch_ctx_ptr->channel_id(), ch_ctx_ptr);
			std::pair<decltype(_channel_contexts)::iterator, bool> ret;
			try {
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ret = _channel_contexts.emplace(pair);
			} catch (...) {
				decltype(_channel_contexts)::iterator old;
				{
					std::lock_guard<std::mutex> lock(_shared_mutex);
					if ((old = _channel_contexts.find(ch_ctx_ptr->channel_id())) != _channel_contexts.end()) {
						old->second->close_immediately(); // close old context;
						_channel_contexts.erase(ch_ctx_ptr->channel_id());
					}
				}
				ret.second = false;
			}

			if (!ret.second) { // not inserted. already exist. this is a critical error!
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("critical error. duplicate id for new channel."));
				ch_ctx_ptr->close_immediately();
			}
			else { // successfully inserted.
				ch_ctx_ptr->handler_strand().post([this, local_port, remote_address, remote_port, v, ch_ctx_ptr](){
					call_channel_created(ch_ctx_ptr);

					ch_ctx_ptr->open(v);
					ch_ctx_ptr->bind(local_port);
					if ((remote_address.length() == 0) || (remote_port == 0)) {
						fire_channel_bound(ch_ctx_ptr, true, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
					}
					else {
						fire_channel_bound(ch_ctx_ptr, false, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));

						asio::ip::udp::resolver::query query(remote_address, std::to_string(remote_port));
						_resolver.async_resolve(query, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::resolve_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
					}
				});
			}
		}
		else {
			throw service_error("the service is not running.");
		}
	}

	void bind_connect(uint16_t local_port, ip_port destination, ip v = ip::v4) {
		if (_stat == RUNNING) {
			udp_channel_context_ptr ch_ctx_ptr = std::make_shared<udp_channel_context>(std::make_shared<udp_channel>(asio::ip::udp::socket(_io_service), _buffer_base_size), this);


			auto pair = std::make_pair(ch_ctx_ptr->channel_id(), ch_ctx_ptr);
			std::pair<decltype(_channel_contexts)::iterator, bool> ret;
			try {
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ret = _channel_contexts.emplace(pair);
			} catch (...) {
				decltype(_channel_contexts)::iterator old;
				{
					std::lock_guard<std::mutex> lock(_shared_mutex);
					if ((old = _channel_contexts.find(ch_ctx_ptr->channel_id())) != _channel_contexts.end()) {
						old->second->close_immediately(); // close old context;
						_channel_contexts.erase(ch_ctx_ptr->channel_id());
					}
				}
				ret.second = false;
			}

			if (!ret.second) { // not inserted. already exist. this is a critical error!
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("critical error. duplicate id for new channel."));
				ch_ctx_ptr->close_immediately();
			}
			else { // successfully inserted.
				ch_ctx_ptr->handler_strand().post([this, local_port, destination, v, ch_ctx_ptr](){
					call_channel_created(ch_ctx_ptr);

					ch_ctx_ptr->open(v);
					ch_ctx_ptr->bind(local_port);
					if (!destination.is_valid()) {
						fire_channel_bound(ch_ctx_ptr, true, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
					}
					else { // destination is already resolved. don't need to resolve address.
						fire_channel_bound(ch_ctx_ptr, false, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));

						asio::ip::udp::resolver::iterator end;
						// call base's connect method.
						static_cast<udp_channel_context_base*>(ch_ctx_ptr.get())->connect(destination, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::connect_complete, this, std::placeholders::_1, end, ch_ctx_ptr)));
					}
				});
			}
		}
		else {
			throw service_error("the service is not running.");
		}
	}

	void write_channel_to(std::size_t channel_id, const ip_port& destination, object_ptr data) {
		if (_stat == RUNNING) {
			try {
				typename decltype(_channel_contexts)::mapped_type ch_ctx_ptr;
				{
					std::lock_guard<std::mutex> lock(_shared_mutex);
					ch_ctx_ptr = std::static_pointer_cast<udp_channel_context>(_channel_contexts.at(channel_id));
				}
				fire_channel_write_to(ch_ctx_ptr, data, destination);
			} catch (const std::exception& e) {
				throw service_error(e.what());
			}
		}
		else {
			throw service_error("the service is not running.");
		}
	}

private:
	virtual void stop() override {
		service_base::stop(); // no more handler will be called.
		
		_service_cv.notify_all();
	}

	void resolve_complete(const asio::error_code& error, asio::ip::udp::resolver::iterator it, udp_channel_context_ptr ch_ctx_ptr) {
		if (error) {
			fire_channel_exception_caught(ch_ctx_ptr, resolver_error("fail to resolve address"));
			_channel_contexts.erase(ch_ctx_ptr->channel_id());
		}
		else {
			ch_ctx_ptr->set_task_in_progress(T_CONNECT);
			ch_ctx_ptr->connect(it, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::connect_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
		}
	}

	void connect_complete(const asio::error_code& error, asio::ip::udp::resolver::iterator it, udp_channel_context_ptr ch_ctx_ptr) {
		// if 'it' parameter is default constucted, it means that connect() was called without resolving address.
		ch_ctx_ptr->clear_task_in_progress(T_CONNECT);

		if (error) {
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(error.message()));
			_channel_contexts.erase(ch_ctx_ptr->channel_id());
		}
		else {
			call_channel_connected(ch_ctx_ptr, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
		}
	}

	void fire_channel_bound(udp_channel_context_ptr ch_ctx_ptr, bool call_read, std::function<read_complete_handler> read_handler) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, call_read, read_handler]() {
			if (!ch_ctx_ptr->settable_stat(context_stat::BOUND))
				return;
			ch_ctx_ptr->stat(context_stat::BOUND);

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

			if (call_read) {
				if (ch_ctx_ptr->stat() == BOUND) {
					ch_ctx_ptr->set_task_in_progress(T_READ);
					ch_ctx_ptr->read(read_handler);
				}
			}
		});
	}

	void call_channel_connected(udp_channel_context_ptr ch_ctx_ptr, std::function<read_complete_handler> read_handler) {
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
				(*it)->channel_connected(ch_ctx_ptr.get());
		} catch (const std::exception& e) {
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_connected handler. (") + e.what() + ")"));
		}

		if (ch_ctx_ptr->stat() == CONNECTED) {
			ch_ctx_ptr->set_task_in_progress(T_READ);
			ch_ctx_ptr->read(read_handler);
		}

		ch_ctx_ptr->handle_req_close(ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::shutdown_complete, this, std::placeholders::_1, ch_ctx_ptr)));
	}

	virtual void call_channel_read(udp_channel_context_base_ptr ch_ctx_ptr, std::size_t bytes_transferred, std::function<read_complete_handler> read_handler) override {
		auto ep = static_cast<udp_channel_context*>(ch_ctx_ptr.get())->sender();
		ip_port ip(ep.address(), ep.port());

		util::streambuf_ptr sb_ptr = ch_ctx_ptr->read_buf();
		sb_ptr->commit(bytes_transferred);

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

		if ((ch_ctx_ptr->stat() == BOUND) || (ch_ctx_ptr->stat() == CONNECTED)) {
			ch_ctx_ptr->set_task_in_progress(T_READ);
			ch_ctx_ptr->read(read_handler);
		}
	}

	virtual void fire_channel_write(udp_channel_context_base_ptr ch_ctx_ptr, object_ptr data) override {
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
				if ((ch_ctx_ptr->stat() == BOUND) || (ch_ctx_ptr->stat() == CONNECTED)) {
					ch_ctx_ptr->set_task_in_progress(T_WRITE);
					ch_ctx_ptr->write_streambuf(sb_ptr, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::write_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
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

	void fire_channel_write_to(udp_channel_context_base_ptr ch_ctx_ptr, object_ptr data, const ip_port& destination) {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr, data, destination]() {
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
				if ((ch_ctx_ptr->stat() == BOUND) || (ch_ctx_ptr->stat() == CONNECTED)) {
					ch_ctx_ptr->set_task_in_progress(T_WRITE);
					static_cast<udp_channel_context*>(ch_ctx_ptr.get())->write_streambuf_to(sb_ptr, destination, ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::write_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
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

	virtual void call_channel_written(udp_channel_context_base_ptr ch_ctx_ptr, std::size_t bytes_transferred) override {
		if ((ch_ctx_ptr->stat() == context_stat::BOUND) || (ch_ctx_ptr->stat() == context_stat::CONNECTED)) { // if channel_disconnected() is already called, don't call channel_written().
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_written(static_cast<udp_channel_context*>(ch_ctx_ptr.get()), bytes_transferred);
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_written handler. (") + e.what() + ")"));
			}
		}
		
		if ((ch_ctx_ptr->stat() == context_stat::BOUND) || (ch_ctx_ptr->stat() == context_stat::CONNECTED)) {
			if (ch_ctx_ptr->has_data_to_write()) {
				ch_ctx_ptr->set_task_in_progress(T_WRITE);
				ch_ctx_ptr->write_next_streambuf(ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::write_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
			}
		}
	}

	virtual void call_channel_disconnected(udp_channel_context_base_ptr ch_ctx_ptr) override {
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
					(*it)->channel_disconnected(static_cast<udp_channel_context*>(ch_ctx_ptr.get()));
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

	virtual void fire_channel_close(udp_channel_context_base_ptr ch_ctx_ptr) override {
		if ((ch_ctx_ptr->stat() >= context_stat::OPEN) && (ch_ctx_ptr->stat() < context_stat::REQ_CLOSE)) {
			ch_ctx_ptr->stat(context_stat::REQ_CLOSE); // must change stat to REQ_CLOSE instantly to prevent different operations from being fired.

			ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr](){
				if (ch_ctx_ptr->stat() == context_stat::REQ_CLOSE) {
					if (ch_ctx_ptr->is_channel_busy())
						ch_ctx_ptr->cancel_all(); // some complete handlers(read_complete, write_complete, ...) will be called.
					else
						ch_ctx_ptr->close_gracefully(ch_ctx_ptr->handler_strand().wrap(std::bind(&udp_service::shutdown_complete, this, std::placeholders::_1, ch_ctx_ptr)));
				}
			});
		}
	}

private:
	asio::ip::udp::resolver _resolver;
};

} // namespace net
} // namespace shan

#endif /* shan_net_udp_service_h */
