//
//  tcp_server_base.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 5..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_server_base_h
#define shan_net_tcp_server_base_h

namespace shan {
namespace net {

class tcp_server_base : public service_base<protocol::tcp> {
	friend class acceptor_context;
public:
	tcp_server_base(std::size_t worker_count = 4, std::size_t buffer_base_size = default_buffer_base_size)
	: service_base<protocol::tcp>(worker_count, buffer_base_size)
	, _acceptor_pipeline_ptr(new acceptor_pipeline()) {}

	virtual ~tcp_server_base() {
		// Note: service's destructor must not be called in worker thread(channel handler)
		std::lock_guard<std::mutex> lock(_service_mutex);
		if (_stat == RUNNING)
			stop();
	}

	void add_acceptor_handler(acceptor_handler* ac_handler_p) {
		add_acceptor_handler(acceptor_handler_ptr(ac_handler_p));
	}

	void add_acceptor_handler(acceptor_handler_ptr ac_handler_ptr) {
		std::lock_guard<std::mutex> lock(_shared_mutex);
		if (_stat == CREATED)
			_acceptor_pipeline_ptr->add_handler_ptr(ac_handler_ptr);
		else
			throw service_error("the service is already started");
	}

	typename acceptor_pipeline::handler_ptr get_acceptor_handler_ptr(std::size_t index) {
		return _acceptor_pipeline_ptr->get_handler_ptr(index);
	}

	void start(uint16_t port, bool reuse_addr = true, int listen_backlog = DEFAULT_BACKLOG, ip v = ip::v4) {
		try {
			std::lock_guard<std::mutex> lock(_service_mutex);
			if (_stat == CREATED) {
				service_base::start();

				_acceptor_context_ptr = std::make_shared<acceptor_context>(acceptor(_io_service, v));
				_acceptor_context_ptr->start(port, reuse_addr, listen_backlog);

				prepare_new_channel_for_next_accept();
				_acceptor_context_ptr->accept(socket_of_new_channel(), std::bind(&tcp_server_base::accept_complete, this, std::placeholders::_1, std::placeholders::_2)); // don't have to wrap handler
			}
		} catch (const std::exception& e) {
			_acceptor_context_ptr->call_exception_caught(this, acceptor_error(e.what()));
		}
	}

protected:
	virtual void prepare_new_channel_for_next_accept() = 0;
	virtual asio::ip::tcp::socket& socket_of_new_channel() = 0; // return the asio socket of the prepared channel.
	virtual tcp_channel_context_base_ptr new_channel_context() = 0;
	virtual void new_channel_accepted(tcp_channel_context_base_ptr ch_ctx_ptr) = 0;
	
	virtual void stop() override {
		service_base::stop(); // no more handler will be called.
		
		_acceptor_context_ptr->stop();
		_acceptor_context_ptr = nullptr; // release ownership
		
		_service_cv.notify_all();
	}

	const std::vector<acceptor_pipeline::handler_ptr>& acceptor_handlers() const {
		return _acceptor_pipeline_ptr->handlers();
	}

	void accept_complete(const asio::error_code& error, const asio::ip::tcp::endpoint& peer_endpoint) {
		if (error) {
			if (error == asio::error::operation_aborted)
				return; // acceptor closed.
			else
				_acceptor_context_ptr->call_exception_caught(this, acceptor_error(error.message()));
		}
		else {
			auto ch_ctx_ptr = new_channel_context();
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
				ch_ctx_ptr->fire_exception_caught(channel_error("critical error. duplicate id for new channel. or not enough memory."));
				ch_ctx_ptr->close_immediately();
			}
			else { // successfully inserted
				_acceptor_context_ptr->call_channel_accepted(this, peer_endpoint);
				new_channel_accepted(ch_ctx_ptr);
			}
		}

		// accept next client
		if (_acceptor_context_ptr->stat() == context_stat::STARTED) {
			prepare_new_channel_for_next_accept();
			_acceptor_context_ptr->accept(socket_of_new_channel(), std::bind(&tcp_server_base::accept_complete, this, std::placeholders::_1, std::placeholders::_2)); // don't have to wrap handler
		}
	}

protected:
	acceptor_context_ptr _acceptor_context_ptr;
	acceptor_pipeline_ptr _acceptor_pipeline_ptr;
};

} // namespace net
} // namespace shan

#include "tcp_server.h"
#ifdef SHAN_NET_SSL_ENABLE
#include "ssl_server.h"
#endif

#endif /* shan_net_tcp_server_base_h */
