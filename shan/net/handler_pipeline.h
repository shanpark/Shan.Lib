//
//  handler_pipeline.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_handler_pipeline_h
#define shan_net_handler_pipeline_h

namespace shan {
namespace net {

template<typename Handler>
class handler_pipeline : public object {
public:
	using handler_ptr = std::unique_ptr<Handler>;
	using ptr = std::unique_ptr<handler_pipeline<Handler>>;

public:
	void add_handler(handler_ptr h) {
		_handlers.push_back(std::move(h));
	}

	const std::vector<handler_ptr>& handlers() {
		return _handlers;
	}
	
private:
	std::vector<handler_ptr> _handlers;
};

using acceptor_handler_ptr = handler_pipeline<acceptor_handler>::handler_ptr;
using acceptor_pipeline = handler_pipeline<acceptor_handler>;
using acceptor_pipeline_ptr =  acceptor_pipeline::ptr;

template<typename Protocol>
using channel_pipeline = handler_pipeline<channel_handler<Protocol>>;
template<typename Protocol>
using channel_pipeline_ptr = typename handler_pipeline<channel_handler<Protocol>>::ptr;

using tcp_channel_handler_ptr = handler_pipeline<tcp_channel_handler>::handler_ptr;
using tcp_channel_pipeline = channel_pipeline<protocol::tcp>;
using tcp_channel_pipeline_ptr = channel_pipeline_ptr<protocol::tcp>;

using udp_channel_handler_ptr = handler_pipeline<udp_channel_handler>::handler_ptr;
using udp_channel_pipeline = channel_pipeline<protocol::udp>;
using udp_channel_pipeline_ptr = channel_pipeline_ptr<protocol::tcp>;


} // namespace net
} // namespace shan

#endif /* shan_net_handler_pipeline_h */
