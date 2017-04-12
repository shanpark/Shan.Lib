//
//  udp_channel_context.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 29..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_udp_channel_context_h
#define shan_net_udp_channel_context_h

namespace shan {
namespace net {

class udp_channel_context : public udp_channel_context_base {
	friend class udp_service;
public:
	udp_channel_context(udp_channel_ptr ch_ptr, service_base<protocol::udp>* svc_p)
	: channel_context<protocol::udp>(ch_ptr->io_service(), svc_p), _channel_ptr(std::move(ch_ptr)) {}

	void write_to(const ip_port& destination, object_ptr data);

private:
	void bind(uint16_t port, ip v = ip::v4) {
		_channel_ptr->bind(port, v);
	}

	bool write_streambuf_to(util::streambuf_ptr write_sb_ptr, const ip_port& destination, std::function<write_complete_handler> write_handler) {
		if ((stat() >= OPEN) && (stat() < DISCONNECTED)) {
			_channel_ptr->write_streambuf_to(write_sb_ptr, destination, write_handler);
			return true;
		}

		return false;
	}

	asio::ip::udp::endpoint& sender() {
		return _channel_ptr->sender();
	}

	virtual channel_base<protocol::udp>* channel_p() override {
		return _channel_ptr.get();
	}

	virtual void close_gracefully(std::function<shutdown_complete_handler> shutdown_handler) noexcept override {
		_channel_ptr->close_gracefully(shutdown_handler);
		shutdown_handler(false);
	}

	void connect(asio::ip::udp::resolver::iterator it, std::function<udp_connect_complete_handler> connect_handler) {
		handler_strand().post([this, it, connect_handler](){
			_channel_ptr->connect(it, connect_handler);
		});
	}

private:
	udp_channel_ptr _channel_ptr;
};

using udp_channel_context_ptr = std::shared_ptr<udp_channel_context>;

} // namespace net
} // namespace shan

#include "service_base.h"

namespace shan {
namespace net {

inline void udp_channel_context::write_to(const ip_port& destination, object_ptr data) {
	static_cast<udp_service*>(_service_p)->write_channel_to(channel_id(), destination, data);
}

} // namespace net
} // namespace shan

#endif /* shan_net_udp_channel_context_h */
