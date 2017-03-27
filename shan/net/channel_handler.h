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

class channel_handler : public handler {
public:
	virtual void channel_connected(channel_context* ctx) {} // <-- inbound
	virtual void channel_read(channel_context* ctx, object_ptr& data) {} // <-- inbound
	virtual void channel_rdbuf_empty(channel_context* ctx) {} // <-- inbound
	virtual void channel_write(channel_context* ctx, object_ptr& data) {} // outbound -->
	virtual void channel_written(channel_context* ctx, std::size_t bytes_transferred, util::streambuf_ptr sb_ptr) {} // <-- inbound
	virtual void channel_disconnected(channel_context* ctx) {} // <-- inbound
};

} // namespace net
} // namespace shan

#endif /* shan_net_channel_handler_h */
