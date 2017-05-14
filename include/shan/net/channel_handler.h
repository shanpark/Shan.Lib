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

template<typename Protocol>
class channel_base;

class tcp_channel_base;
using udp_channel_base = channel_base<protocol::udp>;

template<typename Protocol>
class channel_context;

class tcp_channel_context_base;
using udp_channel_context_base = channel_context<protocol::udp>;

class udp_channel_context;

template<typename Protocol>
class channel_handler; // no body implementation.

template<>
class channel_handler<protocol::tcp> : public object {
public:
	virtual void channel_created(shan::net::tcp_channel_context_base* ctx, tcp_channel_base* channel) {} // <-- inbound
	virtual void channel_connected(shan::net::tcp_channel_context_base* ctx) {} // <-- inbound
	virtual void channel_read(shan::net::tcp_channel_context_base* ctx, shan::object_ptr& data) {} // <-- inbound
	virtual void channel_rdbuf_empty(shan::net::tcp_channel_context_base* ctx) {} // <-- inbound
	virtual void channel_write(shan::net::tcp_channel_context_base* ctx, shan::object_ptr& data) {} // outbound -->
	virtual void channel_written(shan::net::tcp_channel_context_base* ctx, std::size_t bytes_transferred) {} // <-- inbound
	virtual void channel_disconnected(shan::net::tcp_channel_context_base* ctx) {} // <-- inbound
	
	virtual void user_event(shan::net::tcp_channel_context_base* ctx, int64_t id, shan::object_ptr data_ptr) {} // <-- inbound
	virtual void exception_caught(shan::net::tcp_channel_context_base* ctx, const std::exception& e) {} // <-- inbound
};

template<>
class channel_handler<protocol::udp> : public object {
public:
	virtual void channel_created(shan::net::udp_channel_context* ctx, udp_channel_base* channel) {} // <-- inbound
	virtual void channel_bound(shan::net::udp_channel_context* ctx) {} // inbound
	virtual void channel_connected(shan::net::udp_channel_context* ctx) {} // inbound
	virtual void channel_read_from(shan::net::udp_channel_context* ctx, shan::object_ptr& data, const shan::net::ip_port& from) {} // inbound
	virtual void channel_write(shan::net::udp_channel_context* ctx, shan::object_ptr& data) {} // outbound
	virtual void channel_written(shan::net::udp_channel_context* ctx, std::size_t bytes_transferred) {} // inbound
	virtual void channel_disconnected(shan::net::udp_channel_context* ctx) {} // inbound

	virtual void user_event(shan::net::udp_channel_context* ctx, int64_t id, shan::object_ptr data_ptr) {} // <-- inbound
	virtual void exception_caught(shan::net::udp_channel_context* ctx, const std::exception& e) {} // <-- inbound
};

template<typename Protocol>
using channel_handler_ptr = std::shared_ptr<channel_handler<Protocol>>;

template<typename Protocol>
using handler_vector = std::vector<channel_handler_ptr<Protocol>>;

using tcp_channel_handler = channel_handler<protocol::tcp>;
using udp_channel_handler = channel_handler<protocol::udp>;

} // namespace net
} // namespace shan

#endif /* shan_net_channel_handler_h */
