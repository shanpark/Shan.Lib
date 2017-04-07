//
//  tcp_channel_context.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 29..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_channel_context_h
#define shan_net_tcp_channel_context_h

namespace shan {
namespace net {

class tcp_channel_context : public tcp_channel_context_base {
	friend class tcp_client_base;
public:
	tcp_channel_context(tcp_channel_ptr ch_ptr, service_base<protocol::tcp>* svc_p)
	: channel_context<protocol::tcp>(ch_ptr->io_service(), svc_p), _channel_ptr(std::move(ch_ptr)) {}

private:
	virtual channel_base<protocol::tcp>* channel_p() override {
		return _channel_ptr.get();
	}

	void connect(asio::ip::tcp::resolver::iterator it, std::function<tcp_connect_complete_handler> connect_handler) {
		asio::async_connect(static_cast<tcp_channel*>(channel_p())->socket(), it, connect_handler);
	}

private:
	tcp_channel_ptr _channel_ptr;
};

//using tcp_channel_context_ptr = std::shared_ptr<tcp_channel_context>;

} // namespace net
} // namespace shan

#endif /* shan_net_tcp_channel_context_h */
