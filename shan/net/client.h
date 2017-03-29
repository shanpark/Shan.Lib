//
//  client.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_client_h
#define shan_net_client_h

namespace shan {
namespace net {

class client : public tcp_service {
public:
	client(std::size_t worker_count = 2, std::size_t buffer_base_size = default_buffer_base_size)
	: tcp_service(worker_count, buffer_base_size) {}

	virtual ~client() { stop(); }

	virtual void start() noexcept {
		std::lock_guard<std::mutex> lock(_mutex);
		if (!is_running()) {
			service::start();

			_resolver_ptr = std::unique_ptr<asio::ip::tcp::resolver>(new asio::ip::tcp::resolver(*_io_service_ptr));
		}
	}

	virtual void stop() noexcept {
		std::lock_guard<std::mutex> lock(_mutex);
		if (is_running()) {
			service::stop(); // no more handler will be called.

			_resolver_ptr = nullptr; // release ownership
			_cv.notify_all();
		}
	}

	void connect(const std::string& address, uint16_t port) {
		if (is_running()) {
			asio::ip::tcp::resolver::query query(address, std::to_string(port));
			channel_context_ptr ch_ctx_ptr = std::make_shared<tcp_channel_context>(tcp_channel_ptr(new tcp_channel(asio::ip::tcp::socket(*_io_service_ptr), _buffer_base_size)), this);

			_resolver_ptr->async_resolve(query, std::bind(&client::resolve_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
		}
	}

private:
	virtual bool is_running() { return static_cast<bool>(_resolver_ptr); }

	void resolve_complete(const asio::error_code& error, asio::ip::tcp::resolver::iterator it, channel_context_ptr ch_ctx_ptr) {
		if (error) {
			fire_channel_exception_caught(ch_ctx_ptr, resolver_error("fail to resolve address"));
		}
		else {
			try {
				// a successful resolve operation is guaranteed to pass at least one entry to the handler
				ch_ctx_ptr->open(it->endpoint().protocol() == asio::ip::tcp::v6() ? ip::v6 : ip::v4); // opened channel can only have an id.
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(e.what()));
				return;
			}

			auto pair = std::make_pair(ch_ctx_ptr->channel_id(), ch_ctx_ptr);
			std::pair<std::unordered_map<std::size_t, channel_context_ptr>::iterator, bool> ret;
			try {
				std::lock_guard<std::mutex> lock(_mutex);
				ret = _channel_contexts.emplace(pair);
			} catch (...) {
				ret.second = false;
			}
			if (!ret.second) { // not inserted. already exist. this is a critical error!
				fire_channel_exception_caught(ch_ctx_ptr, channel_error("critical error. duplicate id for new channel."));
				ch_ctx_ptr->close_immediately();
			}
			else {
				ch_ctx_ptr->connect(it->endpoint().address().to_string(), it->endpoint().port(), std::bind((&client::connect_complete), this, std::placeholders::_1, ch_ctx_ptr)); // try first endpoint unconditionally.
			}
		}
	}

	void connect_complete(const asio::error_code& error, channel_context_ptr ch_ctx_ptr) {
		if (error)
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(error.message()));
		else
			fire_channel_connected(ch_ctx_ptr, std::bind(&client::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
	}

private:
	std::unique_ptr<asio::ip::tcp::resolver> _resolver_ptr;
};

} // namespace net
} // namespace shan

#endif /* shan_net_client_h */
