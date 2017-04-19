//
//  tcp_client.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_client_h
#define shan_net_tcp_client_h

namespace shan {
namespace net {

class tcp_client : public tcp_client_base {
public:
	tcp_client(std::size_t worker_count = 2, std::size_t buffer_base_size = default_buffer_base_size)
	: tcp_client_base(worker_count, buffer_base_size) {}

private:
	virtual tcp_channel_context_base_ptr new_channel_context() override {
		return std::make_shared<tcp_channel_context>(std::make_shared<tcp_channel>(asio::ip::tcp::socket(_io_service), _buffer_base_size), this);
	}

	virtual void new_channel_connected(tcp_channel_context_base_ptr ch_ctx_ptr) override {
		call_channel_connected(ch_ctx_ptr, ch_ctx_ptr->handler_strand().wrap(std::bind(&tcp_client::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
	}

	virtual void resolve_complete(const asio::error_code& error, asio::ip::tcp::resolver::iterator it, tcp_channel_context_base_ptr ch_ctx_ptr) override {
		if (error) {
			fire_channel_exception_caught(ch_ctx_ptr, resolver_error("fail to resolve address"));
		}
		else {
			// be called in resolve_complete.
			ch_ctx_ptr->set_task_in_progress(T_CONNECT);
			static_cast<tcp_channel_context*>(ch_ctx_ptr.get())->connect(it, ch_ctx_ptr->handler_strand().wrap(std::bind(&tcp_client::connect_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
		}
	}
};

} // namespace net
} // namespace shan

#endif /* shan_net_tcp_client_h */
