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
				ch_ctx_ptr->fire_exception_caught(channel_error("critical error. duplicate id for new channel."));
				ch_ctx_ptr->close_immediately();
			}
			else { // successfully inserted.
				ch_ctx_ptr->strand().post([this, local_port, remote_address, remote_port, v, ch_ctx_ptr](){
					ch_ctx_ptr->call_channel_created();

					ch_ctx_ptr->open(v);
					ch_ctx_ptr->bind(local_port);
					if ((remote_address.length() == 0) || (remote_port == 0)) {
						ch_ctx_ptr->call_channel_bound(true, ch_ctx_ptr->strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
					}
					else {
						ch_ctx_ptr->call_channel_bound(false, ch_ctx_ptr->strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));

						asio::ip::udp::resolver::query query(remote_address, std::to_string(remote_port));
						_resolver.async_resolve(query, ch_ctx_ptr->strand().wrap(std::bind(&udp_service::resolve_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
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
				ch_ctx_ptr->fire_exception_caught(channel_error("critical error. duplicate id for new channel."));
				ch_ctx_ptr->close_immediately();
			}
			else { // successfully inserted.
				ch_ctx_ptr->strand().post([this, local_port, destination, v, ch_ctx_ptr](){
					ch_ctx_ptr->call_channel_created();

					ch_ctx_ptr->open(v);
					ch_ctx_ptr->bind(local_port);
					if (!destination.is_valid()) {
						ch_ctx_ptr->call_channel_bound(true, ch_ctx_ptr->strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
					}
					else { // destination is already resolved. don't need to resolve address.
						ch_ctx_ptr->call_channel_bound(false, ch_ctx_ptr->strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));

						asio::ip::udp::resolver::iterator end;
						// call base's connect method.
						static_cast<udp_channel_context_base*>(ch_ctx_ptr.get())->connect(destination, ch_ctx_ptr->strand().wrap(std::bind(&udp_service::connect_complete, this, std::placeholders::_1, end, ch_ctx_ptr)));
					}
				});
			}
		}
		else {
			throw service_error("the service is not running.");
		}
	}

	void write_channel_to(std::size_t channel_id, const ip_port& destination, object_ptr data_ptr) {
		if (_stat == RUNNING) {
			try {
				std::shared_ptr<udp_channel_context> ch_ctx_ptr;
				{
					std::lock_guard<std::mutex> lock(_shared_mutex);
					ch_ctx_ptr = std::static_pointer_cast<udp_channel_context>(_channel_contexts.at(channel_id));
				}
				ch_ctx_ptr->fire_channel_write_to(data_ptr, destination);
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
			ch_ctx_ptr->fire_exception_caught(resolver_error("fail to resolve address"));
			_channel_contexts.erase(ch_ctx_ptr->channel_id());
		}
		else {
			ch_ctx_ptr->set_task_in_progress(T_CONNECT);
			ch_ctx_ptr->connect(it, ch_ctx_ptr->strand().wrap(std::bind(&udp_service::connect_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
		}
	}

	void connect_complete(const asio::error_code& error, asio::ip::udp::resolver::iterator it, udp_channel_context_ptr ch_ctx_ptr) {
		// if 'it' parameter is default constucted, it means that connect() was called without resolving address.
		ch_ctx_ptr->clear_task_in_progress(T_CONNECT);

		if (error) {
			ch_ctx_ptr->fire_exception_caught(channel_error(error.message()));
			_channel_contexts.erase(ch_ctx_ptr->channel_id());
		}
		else {
			ch_ctx_ptr->call_channel_connected(ch_ctx_ptr->strand().wrap(std::bind(&udp_service::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
		}
	}

private:
	asio::ip::udp::resolver _resolver;
};

} // namespace net
} // namespace shan

#endif /* shan_net_udp_service_h */
