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

class tcp_server_base : public tcp_service_base {
public:
	tcp_server_base(std::size_t worker_count = 4, std::size_t buffer_base_size = default_buffer_base_size)
	: tcp_service_base(worker_count, buffer_base_size)
	, _acceptor_pipeline_ptr(new acceptor_pipeline()) {}

	virtual ~tcp_server_base() {
		stop();
		_join_worker_threads(false);
	}

	bool add_acceptor_handler(acceptor_handler* ac_handler_p) {
		return add_acceptor_handler(acceptor_handler_ptr(ac_handler_p));
	}

	bool add_acceptor_handler(acceptor_handler_ptr ac_handler_ptr) {
		std::lock_guard<std::mutex> lock(_shared_mutex);
		if (!is_running()) {
			_acceptor_pipeline_ptr->add_handler(std::move(ac_handler_ptr));
			return true;
		}

		return false; // if service started, can't add handler.
	}

	void start(uint16_t port, bool reuse_addr = true, int listen_backlog = DEFAULT_BACKLOG, ip v = ip::v4) {
		try {
			std::lock_guard<std::mutex> lock(_service_mutex);
			if (!is_running()) {
				service_base::start();

				_acceptor_context_ptr = acceptor_context_ptr(new acceptor_context(acceptor_ptr(new acceptor(*_io_service_ptr, v))));
				_acceptor_context_ptr->start(port, reuse_addr, listen_backlog);

				prepare_channel_for_next_accept();
				_acceptor_context_ptr->accept(socket(), std::bind(&tcp_server_base::accept_complete, this, std::placeholders::_1, std::placeholders::_2));
			}
		} catch (const std::exception& e) {
			fire_acceptor_exception_caught(acceptor_error(e.what()));
		}
	}

	void stop() {
		while (is_running()) {
			if (_service_mutex.try_lock()) {
				std::lock_guard<std::mutex> lock(_service_mutex, std::adopt_lock);
				if (is_running()) {
					service_base::stop(); // no more handler will be called.

					_acceptor_context_ptr->stop();
					_acceptor_context_ptr = nullptr; // release ownership

					_service_cv.notify_all();
				}
			}
			else {
				std::this_thread::yield();
			}
		}
	}

protected:
	virtual void prepare_channel_for_next_accept() = 0;
	virtual asio::ip::tcp::socket& socket() = 0; // return the asio socket of the prepared channel.
	virtual tcp_channel_context_base_ptr new_channel_context() = 0;
	virtual void new_channel_accepted(tcp_channel_context_base_ptr ch_ctx_ptr) = 0;

	const std::vector<acceptor_pipeline::handler_ptr>& acceptor_handlers() const {
		return _acceptor_pipeline_ptr->handlers();
	}

	void accept_complete(const asio::error_code& error, const asio::ip::tcp::endpoint& peer_endpoint) {
		if (error) {
			if (error == asio::error::operation_aborted)
				return; // acceptor closed.
			else
				fire_acceptor_exception_caught(acceptor_error(error.message()));
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
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("critical error. duplicate id for new channel. or not enough memory."));
				ch_ctx_ptr->close_immediately();
			}
			else { // successfully inserted
				fire_acceptor_channel_accepted(peer_endpoint);
				new_channel_accepted(ch_ctx_ptr);
			}
		}

		// accept next client
		if (_acceptor_context_ptr->stat() == acceptor_context::STARTED) {
			prepare_channel_for_next_accept();
			_acceptor_context_ptr->accept(socket(), std::bind(&tcp_server_base::accept_complete, this, std::placeholders::_1, std::placeholders::_2));
		}
	}

	void fire_acceptor_channel_accepted(const asio::ip::tcp::endpoint& peer_endpoint) {
		acceptor_context* ctx_p = _acceptor_context_ptr.get();
		ctx_p->done(false); // reset context to 'not done'.
		// <-- inbound
		auto begin = acceptor_handlers().begin();
		auto end = acceptor_handlers().end();
		try {
			for (auto it = begin ; !(ctx_p->done()) && (it != end) ; it++)
				(*it)->channel_accepted(ctx_p, peer_endpoint.address().to_string(), peer_endpoint.port());
		} catch (const std::exception& e) {
			fire_acceptor_exception_caught(acceptor_error(std::string("An exception has thrown in channel_accepted handler. (") + e.what() + ")"));
		}
	}

	void fire_acceptor_exception_caught(const acceptor_error& e) {
		context_base* ctx_p = _acceptor_context_ptr.get();
		ctx_p->done(false); // reset context to 'not done'.
		// <-- inbound
		auto begin = acceptor_handlers().begin();
		auto end = acceptor_handlers().end();
		try {
			for (auto it = begin ; !(ctx_p->done()) && (it != end) ; it++)
				(*it)->exception_caught(ctx_p, e);
		} catch (const std::exception& e2) {
			fire_acceptor_exception_caught(acceptor_error(std::string("An exception has thrown in exception_caught handler. (") + e2.what() + ")")); // can cause infinite exception...
		}
	}
	
protected:
	acceptor_context_ptr _acceptor_context_ptr;
	acceptor_pipeline_ptr _acceptor_pipeline_ptr;
};

} // namespace net
} // namespace shan

#endif /* shan_net_tcp_server_base_h */
