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
		return (_stat == RUNNING);
	}

	virtual tcp_channel_context_base_ptr new_channel_context() override {
		return std::make_shared<ssl_channel_context>(std::make_shared<ssl_channel>(_io_service, _ssl_context, _buffer_base_size), this);
	}

	virtual void new_channel_connected(tcp_channel_context_base_ptr ch_ctx_ptr) override {
		ch_ctx_ptr->set_task_in_progress(T_HANDSHAKE);
		static_cast<ssl_channel_context*>(ch_ctx_ptr.get())->handshake(asio::ssl::stream_base::client,
																	   ch_ctx_ptr->strand().wrap(std::bind(&ssl_client::handshake_complete, this, std::placeholders::_1, ch_ctx_ptr)));
	}

	virtual void resolve_complete(const asio::error_code& error, asio::ip::tcp::resolver::iterator it, tcp_channel_context_base_ptr ch_ctx_ptr) override {
		if (error) {
			ch_ctx_ptr->fire_exception_caught(resolver_error("fail to resolve address"));
		}
		else {
			// be called in resolve_complete.
			ch_ctx_ptr->set_task_in_progress(T_CONNECT);
			static_cast<ssl_channel_context*>(ch_ctx_ptr.get())->connect(it, ch_ctx_ptr->strand().wrap(std::bind(&ssl_client::connect_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
		}
	}

	void handshake_complete(const asio::error_code& error, tcp_channel_context_base_ptr ch_ctx_ptr) {
		ch_ctx_ptr->clear_task_in_progress(T_HANDSHAKE);
		if (error)
			ch_ctx_ptr->fire_exception_caught(channel_error(error.message()));
		else
			ch_ctx_ptr->call_channel_connected(ch_ctx_ptr->strand().wrap(std::bind(&ssl_client::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
	}
};

} // namespace net
} // namespace shan

#endif /* shan_net_ssl_client_h */
