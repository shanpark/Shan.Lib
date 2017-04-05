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
	virtual channel_base<protocol::tcp>* channel_p() {
		return _channel_ptr.get();
	}

	void handshake(asio::ssl::stream_base::handshake_type type, const std::function<handshake_complete_hander>& handshake_handler) {
		_channel_ptr->handshake(type, handshake_handler);
	}

private:
	ssl_channel_ptr _channel_ptr;
};

using ssl_channel_context_ptr = std::shared_ptr<ssl_channel_context>;

} // namespace net
} // namespace shan

#endif /* shan_net_ssl_channel_context_h */
