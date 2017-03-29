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

class udp_channel_context : public channel_context {
	friend class udp_service;
public:
	udp_channel_context(udp_channel_ptr ch_ptr, service* svc_p)
	: channel_context(ch_ptr->get_io_service(), svc_p), _channel_ptr(std::move(ch_ptr)) {}

private:
	void bind(uint16_t port, ip v = ip::v4) {
		_channel_ptr->bind(port, v);
	}

	void write_streambuf_to(util::streambuf_ptr write_buf_ptr, const ip_port& destination, std::function<write_complete_handler> write_handler) {
		if (stat() == connected)
			_channel_ptr->write_streambuf_to(write_buf_ptr, destination, write_handler);
//		else //... 뭔가 실패를 리턴해야 한다.
//			ignore
	}

	asio::ip::udp::endpoint& get_sender() {
		return _channel_ptr->get_sender();
	}

	virtual channel* channel_p() { return _channel_ptr.get(); }

private:
	udp_channel_ptr _channel_ptr;
};

using udp_channel_context_ptr = std::shared_ptr<udp_channel_context>;

} // namespace net
} // namespace shan

#include "service.h"

#endif /* shan_net_udp_channel_context_h */
