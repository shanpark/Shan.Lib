//
//  server.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_server_h
#define shan_net_server_h

namespace shan {
namespace net {

class server : public tcp_service {
	friend class acceptor_context;
	
	static const int default_backlog = asio::socket_base::max_connections;

public:
	server(std::size_t worker_count = 4, std::size_t buffer_base_size = default_buffer_base_size)
	: tcp_service(worker_count, buffer_base_size)
	, _acceptor_pipeline_ptr(new acceptor_pipeline()) {}
	
	virtual ~server() { stop(); }

	bool add_acceptor_handler(acceptor_handler* ac_handler_p) {
		return add_acceptor_handler(acceptor_handler_ptr(ac_handler_p));
	}

	bool add_acceptor_handler(acceptor_handler_ptr ac_handler_ptr) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (!is_running()) {
			_acceptor_pipeline_ptr->add_handler(std::move(ac_handler_ptr));
			return true;
		}

		return false; // if service started, can't add handler.
	}

	void start(uint16_t port, bool reuse_addr = true, int listen_backlog = default_backlog, ip v = ip::v4) noexcept {
		try {
			std::lock_guard<std::mutex> lock(_mutex);
			if (!is_running()) {
				service::start();

				_acceptor_context_ptr = acceptor_context_ptr(new acceptor_context(acceptor_ptr(new acceptor(*_io_service_ptr, v)), this));
				_acceptor_context_ptr->start(port, reuse_addr, listen_backlog, std::bind(&server::accept_complete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			}
		} catch (const std::exception& e) {
			fire_acceptor_exception_caught(acceptor_error(e.what()));
		}
	}

	virtual void stop() noexcept {
		std::lock_guard<std::mutex> lock(_mutex);
		if (is_running()) {
			service::stop(); // no more handler will be called.

			_acceptor_context_ptr->stop();
			_acceptor_context_ptr = nullptr; // release ownership

			_cv.notify_all();
		}
	}

private:
	virtual bool is_running() { return static_cast<bool>(_acceptor_context_ptr); }

	const std::vector<acceptor_pipeline::handler_ptr>& acceptor_handlers() const { return _acceptor_pipeline_ptr->handlers(); }

	void accept_complete(const asio::error_code& error, asio::ip::tcp::socket& peer, const asio::ip::tcp::endpoint& peer_endpoint) {
		if (error) {
			if (error == asio::error::operation_aborted)
				return;
			else
				fire_acceptor_exception_caught(acceptor_error(error.message()));
		}
		else {
			auto ch_ctx_ptr = std::make_shared<tcp_channel_context>(tcp_channel_ptr(new tcp_channel(std::move(peer), _buffer_base_size)), this);
			auto pair = std::make_pair(ch_ctx_ptr->channel_id(), ch_ctx_ptr);
			std::pair<std::unordered_map<std::size_t, channel_context_ptr>::iterator, bool> ret;
			try {
				std::lock_guard<std::mutex> lock(_mutex);
				ret = _channel_contexts.emplace(pair);
			} catch (...) {
				ret.second = false;
			}
			if (!ret.second) { // not inserted. already exist. this is a critical error!
				fire_channel_exception_caught(pair.second, channel_error("critical error. duplicate id for new channel. or not enough memory."));
				pair.second->close_immediately();
			}
			else { // successfully inserted
				fire_acceptor_channel_accepted(peer_endpoint);
				fire_channel_connected(pair.second, std::bind(&server::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
			}
		}

		// accept next client
		if (_acceptor_context_ptr->stat() == acceptor_context::started)
			_acceptor_context_ptr->accept(std::bind(&server::accept_complete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	void fire_acceptor_channel_accepted(const asio::ip::tcp::endpoint& peer_endpoint) {
		_acceptor_context_ptr->strand().post([this, peer_endpoint]() {
			acceptor_context* ctx_p = _acceptor_context_ptr.get();
			ctx_p->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = acceptor_handlers().rbegin();
			auto end = acceptor_handlers().rend();
			try {
				for (auto it = begin ; !(ctx_p->done()) && (it != end) ; it++)
					(*it)->channel_accepted(ctx_p, peer_endpoint.address().to_string(), peer_endpoint.port());
			} catch (const std::exception& e) {
				fire_acceptor_exception_caught(acceptor_error(std::string("An exception has thrown in channel_accepted handler. (") + e.what() + ")"));
			}
		});
	}

	void fire_acceptor_exception_caught(const acceptor_error& e) {
		_acceptor_context_ptr->strand().post([this, e]() {
			context* ctx_p = _acceptor_context_ptr.get();
			ctx_p->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = acceptor_handlers().rbegin();
			auto end = acceptor_handlers().rend();
			try {
				for (auto it = begin ; !(ctx_p->done()) && (it != end) ; it++)
					(*it)->exception_caught(ctx_p, e);
			} catch (const std::exception& e2) {
				fire_acceptor_exception_caught(acceptor_error(std::string("An exception has thrown in exception_caught handler. (") + e2.what() + ")")); // can cause infinite exception...
			}
		});
	}

private:
	acceptor_context_ptr _acceptor_context_ptr;
	acceptor_pipeline_ptr _acceptor_pipeline_ptr;
};

} // namespace net
} // namespace shan

#endif /* shan_net_server_h */
