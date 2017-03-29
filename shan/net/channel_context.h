//
//  channel_context.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 17..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_channel_context_h
#define shan_net_channel_context_h

namespace shan {
namespace net {

class channel_context : public context {
	friend class service;
	friend class tcp_service;
	friend class server;
	friend class client;
	friend class udp_service;
public:
	channel_context(asio::io_service& io_service, service* svc_p);
	~channel_context() {
		std::cout << ">>>> channel_context destroyed" << std::endl; //... 삭제 예정.
		streambuf_pool::return_object(_read_streambuf_ptr);
	}

	std::size_t channel_id() { return channel_p()->id(); }

	void write(object_ptr data);
	void close();

protected:
	virtual channel* channel_p() = 0;

	void open(ip v) {
		if (set_stat_if_possible(context::open))
			channel_p()->open(v);
	}

	void close_immediately() noexcept {
		channel_p()->close();
		set_stat_if_possible(closed);
	}

	void connect(const std::string& address, uint16_t port, std::function<connect_complete_handler> connect_handler) {
		channel_p()->connect(address, port, connect_handler);
	}

	void read(std::function<read_complete_handler> read_handler) noexcept {
		channel_p()->read(read_handler);
	}

	void write_streambuf(util::streambuf_ptr write_buf_ptr, std::function<write_complete_handler> write_handler) {
		if (stat() == connected)
			channel_p()->write_streambuf(write_buf_ptr, write_handler);
		//... else ignore.
	}

	util::streambuf_ptr read_buf() { return _read_streambuf_ptr; }

protected:
	util::streambuf_ptr _read_streambuf_ptr;
};

using channel_context_ptr = std::shared_ptr<channel_context>;

} // namespace net
} // namespace shan

#include "tcp_channel_context.h"
#include "udp_channel_context.h"

namespace shan {
namespace net {

inline channel_context::channel_context(asio::io_service& io_service, service* svc_p)
: context(io_service, svc_p), _read_streambuf_ptr(streambuf_pool::get_object(svc_p->_buffer_base_size)) {}

inline void channel_context::write(object_ptr data) {
	_service_p->write_channel(channel_id(), data);
}

inline void channel_context::close() {
	_service_p->close_channel(channel_id());
}

} // namespace net
} // namespace shan

#endif /* shan_net_channel_context_h */
