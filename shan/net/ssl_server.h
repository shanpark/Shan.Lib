//
//  ssl_server.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 3..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_ssl_server_h
#define shan_net_ssl_server_h

namespace shan {
namespace net {

class ssl_server : public tcp_server_base, public ssl_service_base {
public:
	ssl_server(ssl_method method, std::size_t worker_count = 4, std::size_t buffer_base_size = default_buffer_base_size)
	: tcp_server_base(worker_count, buffer_base_size), ssl_service_base(method) {}

private:
	virtual bool is_running() override {
		return (_stat == RUNNING);
	}

	virtual void prepare_new_channel_for_next_accept() override {
		_new_channel = std::make_shared<ssl_channel>(_io_service, _ssl_context, _buffer_base_size);
	}

	virtual asio::ip::tcp::socket& socket_of_new_channel() override { // return the asio socket of the prepared channel.
		return _new_channel->socket();
	}

	virtual tcp_channel_context_base_ptr new_channel_context() override {
		return std::make_shared<ssl_channel_context>(std::move(_new_channel), this);
	}

	virtual void new_channel_accepted(tcp_channel_context_base_ptr ch_ctx_ptr) override {
		ch_ctx_ptr->handler_strand().post([this, ch_ctx_ptr](){
			ch_ctx_ptr->set_task_in_progress(T_HANDSHAKE);
			static_cast<ssl_channel_context*>(ch_ctx_ptr.get())->handshake(asio::ssl::stream_base::server, ch_ctx_ptr->handler_strand().wrap(std::bind(&ssl_server::handshake_complete, this, std::placeholders::_1, ch_ctx_ptr)));
		});
	}

private:
	void handshake_complete(const asio::error_code& error, tcp_channel_context_base_ptr ch_ctx_ptr) {
		ch_ctx_ptr->clear_task_in_progress(T_HANDSHAKE);
		if (error)
			fire_channel_exception_caught(ch_ctx_ptr, channel_error(error.message()));
		else
			call_channel_connected(ch_ctx_ptr, ch_ctx_ptr->handler_strand().wrap(std::bind(&ssl_server::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
	}

private:
	ssl_channel_ptr _new_channel;
};

} // namespace net
} // namespace shan

#endif /* shan_net_ssl_server_h */
