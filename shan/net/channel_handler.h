//
//  channel_handler.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 16..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_channel_handler_h
#define shan_net_channel_handler_h

namespace shan {
namespace net {

class tcp_channel_context;
class udp_channel_context;

template<typename Protocol>
class channel_handler; // no body implementation.

template<>
class channel_handler<protocol::tcp> : public handler {
public:
	virtual void channel_connected(shan::net::tcp_channel_context* ctx) {} // <-- inbound
	virtual void channel_read(shan::net::tcp_channel_context* ctx, shan::object_ptr& data) {} // <-- inbound
	virtual void channel_rdbuf_empty(shan::net::tcp_channel_context* ctx) {} // <-- inbound
	virtual void channel_write(shan::net::tcp_channel_context* ctx, shan::object_ptr& data) {} // outbound -->
	virtual void channel_written(shan::net::tcp_channel_context* ctx, std::size_t bytes_transferred, shan::util::streambuf_ptr sb_ptr) {} // <-- inbound
	virtual void channel_disconnected(shan::net::tcp_channel_context* ctx) {} // <-- inbound
};

template<>
class channel_handler<protocol::udp> : public handler {
public:
	virtual void channel_bound(shan::net::udp_channel_context* ctx) {} // inbound
	virtual void channel_connected(shan::net::udp_channel_context* ctx) {} // inbound
	virtual void channel_read_from(shan::net::udp_channel_context* ctx, shan::object_ptr& data, const shan::net::ip_port& from) {} // inbound
	virtual void channel_write(shan::net::udp_channel_context* ctx, shan::object_ptr& data) {} // outbound
	virtual void channel_written(shan::net::udp_channel_context* ctx, std::size_t bytes_transferred, shan::util::streambuf_ptr sb_ptr) {} // inbound
	virtual void channel_disconnected(shan::net::udp_channel_context* ctx) {} // inbound
};

template<typename Protocol>
using channel_handler_ptr = std::unique_ptr<channel_handler<Protocol>>;

using tcp_channel_handler = channel_handler<protocol::tcp>;
using udp_channel_handler = channel_handler<protocol::udp>;

} // namespace net
} // namespace shan

//#include "tcp_channel_handler.h"
//#include "udp_channel_handler.h"

#endif /* shan_net_channel_handler_h */
