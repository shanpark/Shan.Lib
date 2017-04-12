//
//  ssl_client.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 4..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_ssl_client_h
#define shan_net_ssl_client_h

namespace shan {
namespace net {

class ssl_client : public tcp_client_base, public ssl_service_base {
public:
	ssl_client(ssl_method method, std::size_t worker_count = 2, std::size_t buffer_base_size = default_buffer_base_size)
	: tcp_client_base(worker_count, buffer_base_size), ssl_service_base(method) {}

private:
	virtual bool is_running() override {
		return tcp_client_base::is_running();
	}

	virtual tcp_channel_context_base_ptr new_channel_context() override {
		return std::make_shared<ssl_channel_context>(ssl_channel_ptr(new ssl_channel(*_io_service_ptr, _ssl_context, _buffer_base_size)), this);
	}

	virtual void new_channel_connected(tcp_channel_context_base_ptr ch_ctx_ptr) override {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr](){
			static_cast<ssl_channel_context*>(ch_ctx_ptr.get())->handshake(asio::ssl::stream_base::client, std::bind(&ssl_client::handshake_complete, this, std::placeholders::_1, ch_ctx_ptr));
		});
	}

	virtual void resolve_complete(const asio::error_code& error, asio::ip::tcp::resolver::iterator it, tcp_channel_context_base_ptr ch_ctx_ptr) override {
		if (error) {
			fire_channel_exception_caught(ch_ctx_ptr, resolver_error("fail to resolve address"));
		}
		else {
			static_cast<ssl_channel_context*>(ch_ctx_ptr.get())->connect(it, std::bind(&ssl_client::connect_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
		}
	}

	void handshake_complete(const asio::error_code& error, tcp_channel_context_base_ptr ch_ctx_ptr) {
		if (error)
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(error.message()));
		else
			fire_channel_connected(ch_ctx_ptr, std::bind(&ssl_client::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr));
	}
};

} // namespace net
} // namespace shan

#endif /* shan_net_ssl_client_h */
