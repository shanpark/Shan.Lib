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

class channel_context;
class tcp_channel_context;
class udp_channel_context;

class channel_handler : public handler {
public:
	virtual void channel_bound(shan::net::udp_channel_context* ctx) {} // <-- inbound (udp only)
	virtual void channel_connected(shan::net::channel_context* ctx) {} // <-- inbound
	virtual void channel_read(shan::net::tcp_channel_context* ctx, shan::object_ptr& data) {} // <-- inbound (tcp_only)
	virtual void channel_rdbuf_empty(shan::net::tcp_channel_context* ctx) {} // <-- inbound (tcp only)
	virtual void channel_read_from(shan::net::udp_channel_context* ctx, shan::object_ptr& data, const shan::net::ip_port& from) {} // <-- inbound (udp only)
	virtual void channel_write(shan::net::channel_context* ctx, shan::object_ptr& data) {} // outbound -->
	virtual void channel_written(shan::net::channel_context* ctx, std::size_t bytes_transferred, shan::util::streambuf_ptr sb_ptr) {} // <-- inbound
	virtual void channel_disconnected(shan::net::channel_context* ctx) {} // <-- inbound
};

} // namespace net
} // namespace shan

#endif /* shan_net_channel_handler_h */
