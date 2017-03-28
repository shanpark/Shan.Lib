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
public:
	channel_context(channel_ptr ch_ptr, service* svc_p);
	~channel_context() {
		std::cout << ">>>> channel_context destroyed" << std::endl; //... 삭제 예정.
		streambuf_pool::return_object(_read_streambuf_ptr);
	}

	std::size_t channel_id() { return _channel_ptr->id(); }

	void write(object_ptr data);
	void close();

private:
	void channel_open(ip v) {
		if (set_stat_if_possible(open))
			_channel_ptr->open(v);
	}

	void channel_close() noexcept {
		_channel_ptr->close();
		set_stat_if_possible(closed);
	}

	void connect(asio::ip::tcp::endpoint ep, std::function<connect_complete_handler> connect_handler) {
		static_cast<tcp_channel*>(_channel_ptr.get())->connect(ep, connect_handler);
	}

	void read(std::function<read_complete_handler> read_handler) noexcept {
		_channel_ptr->read(read_handler);
	}

	void write_stream(util::streambuf_ptr write_buf_ptr, std::function<write_complete_handler> write_handler) {
		_channel_ptr->write_stream(write_buf_ptr, write_handler);
	}

	util::streambuf_ptr read_buf() { return _read_streambuf_ptr; }

private:
	shan::net::channel_ptr _channel_ptr;
	util::streambuf_ptr _read_streambuf_ptr;
};

using channel_context_ptr = std::shared_ptr<channel_context>;

} // namespace net
} // namespace shan

#include "service.h"

namespace shan {
namespace net {

inline channel_context::channel_context(channel_ptr ch_ptr, service* svc_p)
: context(ch_ptr->get_io_service(), svc_p), _channel_ptr(std::move(ch_ptr))
, _read_streambuf_ptr(streambuf_pool::get_object(svc_p->_buffer_base_size)) {}

inline void channel_context::write(object_ptr data) {
	if (_service_p->is_tcp())
		static_cast<tcp_service*>(_service_p)->write_to(channel_id(), data);
}

inline void channel_context::close() {
	if (_service_p->is_tcp())
		static_cast<tcp_service*>(_service_p)->close_channel(channel_id());
}

} // namespace net
} // namespace shan

#endif /* shan_net_channel_context_h */
