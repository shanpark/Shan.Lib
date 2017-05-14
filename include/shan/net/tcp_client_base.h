//
//  tcp_client_base.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 5..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_client_base_h
#define shan_net_tcp_client_base_h

namespace shan {
namespace net {

class tcp_client_base : public service_base<protocol::tcp> {
public:
	tcp_client_base(std::size_t worker_count = 2, std::size_t buffer_base_size = default_buffer_base_size)
	: service_base<protocol::tcp>(worker_count, buffer_base_size), _resolver(_io_service) {}

	virtual ~tcp_client_base() {
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

	void connect(const std::string& address, uint16_t port) {
		if (_stat == RUNNING) {
			asio::ip::tcp::resolver::query query(address, std::to_string(port));

			auto ch_ctx_ptr = new_channel_context();
			ch_ctx_ptr->strand().post([this, query, ch_ctx_ptr](){
				ch_ctx_ptr->call_channel_created();
				_resolver.async_resolve(query, ch_ctx_ptr->strand().wrap(std::bind(&tcp_client_base::resolve_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
			});
		}
		else {
			throw service_error("the service is not running.");
		}
	}

	void connect(ip_port destination) {
		if (_stat == RUNNING) {
			auto ch_ctx_ptr = new_channel_context();
			ch_ctx_ptr->strand().post([this, destination, ch_ctx_ptr](){
				ch_ctx_ptr->call_channel_created();

				asio::ip::tcp::resolver::iterator end;
				ch_ctx_ptr->set_task_in_progress(T_CONNECT);
				ch_ctx_ptr->connect(destination, ch_ctx_ptr->strand().wrap(std::bind((&tcp_client_base::connect_complete), this, std::placeholders::_1, end, ch_ctx_ptr)));
			});
		}
		else {
			throw service_error("the service is not running.");
		}
	}

protected:
	virtual tcp_channel_context_base_ptr new_channel_context() = 0;
	virtual void new_channel_connected(tcp_channel_context_base_ptr ch_ctx_ptr) = 0;
	virtual void resolve_complete(const asio::error_code& error, asio::ip::tcp::resolver::iterator it, tcp_channel_context_base_ptr ch_ctx_ptr) = 0;
	
	virtual void stop() override {
		service_base::stop(); // no more handler will be called.
		
		_service_cv.notify_all();
	}

	void connect_complete(const asio::error_code& error, asio::ip::tcp::resolver::iterator it, tcp_channel_context_base_ptr ch_ctx_ptr) {
		ch_ctx_ptr->clear_task_in_progress(T_CONNECT);

		if (error) {
			ch_ctx_ptr->fire_exception_caught(channel_error(error.message()));
		}
		else {
			auto pair = std::make_pair(ch_ctx_ptr->channel_id(), ch_ctx_ptr);
			std::pair<decltype(_channel_contexts)::iterator, bool> ret;
			try {
				std::lock_guard<std::mutex> lock(_shared_mutex);
				ret = _channel_contexts.emplace(std::move(pair));
			} catch (...) {
				decltype(_channel_contexts)::iterator old;
				{
					std::lock_guard<std::mutex> lock(_shared_mutex);
					if ((old = _channel_contexts.find(pair.first)) != _channel_contexts.end()) {
						old->second->close_immediately(); // close old context;
						_channel_contexts.erase(pair.first);
					}
				}
				ret.second = false;
			}
			if (!ret.second) { // not inserted. already exist. this is a critical error!
				ch_ctx_ptr->fire_exception_caught(channel_error("critical error. duplicate id for new channel."));
				ch_ctx_ptr->close_immediately();
			}
			else {
				new_channel_connected(ch_ctx_ptr);
			}
		}
	}

protected:
	asio::ip::tcp::resolver _resolver;
};

} // namespace net
} // namespace shan

#include <shan/net/tcp_client.h>
#ifdef SHAN_NET_SSL_ENABLE
#include <shan/net/ssl_client.h>
#endif

#endif /* shan_net_tcp_client_base_h */
