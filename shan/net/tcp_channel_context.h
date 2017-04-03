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

class tcp_channel_context : public channel_context<protocol::tcp> {
public:
	tcp_channel_context(tcp_channel_ptr ch_ptr, service<protocol::tcp>* svc_p)
	: channel_context<protocol::tcp>(ch_ptr->io_service(), svc_p), _channel_ptr(std::move(ch_ptr)) {}

private:
	virtual channel<protocol::tcp>* channel_p() {
		return _channel_ptr.get();
	}

private:
	tcp_channel_ptr _channel_ptr;
};

using tcp_channel_context_ptr = std::shared_ptr<tcp_channel_context>;

} // namespace net
} // namespace shan

#endif /* shan_net_tcp_channel_context_h */
