//
//  ssl_channel_context.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 4..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_ssl_channel_context_h
#define shan_net_ssl_channel_context_h

namespace shan {
namespace net {

class ssl_channel_context : public tcp_channel_context_base {
	friend class ssl_server;
	friend class ssl_client;
public:
	ssl_channel_context(ssl_channel_ptr ch_ptr, service_base<protocol::tcp>* svc_p)
	: tcp_channel_context_base(ch_ptr->io_service(), svc_p), _channel_ptr(std::move(ch_ptr)) {}

private:
	virtual channel_base<protocol::tcp>* channel_p() override {
		return _channel_ptr.get();
	}

	virtual void close_gracefully(std::function<shutdown_complete_handler> shutdown_handler) noexcept override {
		stat(CLOSED); // set stat to closed because socket will be closed in shutdown_handler() unconditionally.
					  // this will prevent ssl_channel from being shutdown twice.
					  // the second call attempts to release SSL object twice. (it will make an unexpected error.)
		_channel_ptr->close_gracefully(shutdown_handler);
	}

	void connect(asio::ip::tcp::resolver::iterator it, std::function<tcp_connect_complete_handler> connect_handler) {
		_channel_ptr->connect(it, connect_handler);
	}

	void handshake(asio::ssl::stream_base::handshake_type type, const std::function<handshake_complete_hander>& handshake_handler) {
		_channel_ptr->handshake(type, handshake_handler);
	}

private:
	ssl_channel_ptr _channel_ptr;
};

} // namespace net
} // namespace shan

#endif /* shan_net_ssl_channel_context_h */
