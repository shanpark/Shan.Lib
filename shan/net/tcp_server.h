//
//  tcp_server.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_server_h
#define shan_net_tcp_server_h

namespace shan {
namespace net {

class tcp_server : public tcp_server_base {
public:
	tcp_server(std::size_t worker_count = 4, std::size_t buffer_base_size = default_buffer_base_size)
	: tcp_server_base(worker_count, buffer_base_size) {}

private:
	virtual void prepare_new_channel_for_next_accept() override {
		_new_channel = tcp_channel_ptr(new tcp_channel(asio::ip::tcp::socket(_io_service), _buffer_base_size));
	}

	virtual asio::ip::tcp::socket& socket_of_new_channel() override { // return the asio socket of the prepared channel.
		return _new_channel->socket();
	}

	virtual tcp_channel_context_base_ptr new_channel_context() override {
		return std::make_shared<tcp_channel_context>(std::move(_new_channel), this);
	}

	virtual void new_channel_accepted(tcp_channel_context_base_ptr ch_ctx_ptr) override {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr](){
			ch_ctx_ptr->call_channel_created();
			
			ch_ctx_ptr->call_channel_connected(ch_ctx_ptr->strand().wrap(std::bind(&tcp_server::read_complete, this, std::placeholders::_1, std::placeholders::_2, ch_ctx_ptr)));
		});
	}

private:
	tcp_channel_ptr _new_channel;
};

} // namespace net
} // namespace shan

#endif /* shan_net_tcp_server_h */
