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

template<typename Protocol>
class service;

template<typename Protocol>
class channel_context : public context {
	friend class service<Protocol>;
	friend class tcp_service;
	friend class server;
	friend class client;
	friend class udp_service;

public:
	using ptr = std::shared_ptr<channel_context<Protocol>>;

public:
	channel_context(asio::io_service& io_service, service<Protocol>* svc_p);
	~channel_context() {
		std::cout << ">>>> channel_context destroyed" << std::endl; //... 삭제 예정.
		streambuf_pool::return_object(_read_sb_ptr);
	}

	std::size_t channel_id() { return channel_p()->id(); }

	void write(object_ptr data);
	void close();

protected:
	virtual channel<Protocol>* channel_p() = 0;

	void open(ip v) {
		if (set_stat_if_possible(OPEN))
			channel_p()->open(v);
	}

	void close_immediately() noexcept {
		channel_p()->close();
		set_stat_if_possible(CLOSED);
	}

	void connect(const std::string& address, uint16_t port, std::function<connect_complete_handler> connect_handler) {
		channel_p()->connect(address, port, connect_handler);
	}

	void read(std::function<read_complete_handler> read_handler) noexcept {
		channel_p()->read(read_handler);
	}

	bool write_streambuf(util::streambuf_ptr write_sb_ptr, std::function<write_complete_handler> write_handler) {
		if (stat() == CONNECTED) {
			channel_p()->write_streambuf(write_sb_ptr, write_handler);
			return true;
		}

		return false;
	}

	util::streambuf_ptr read_buf() {
		return _read_sb_ptr;
	}

protected:
	service<Protocol>* _service_p;
	util::streambuf_ptr _read_sb_ptr;
};

template<typename Protocol>
using channel_context_ptr = typename channel_context<Protocol>::ptr;

} // namespace net
} // namespace shan

#include "tcp_channel_context.h"
#include "udp_channel_context.h"

namespace shan {
namespace net {

template<typename Protocol>
inline channel_context<Protocol>::channel_context(asio::io_service& io_service, service<Protocol>* svc_p)
: context(io_service), _service_p(svc_p), _read_sb_ptr(streambuf_pool::get_object(svc_p->_buffer_base_size)) {}

template<typename Protocol>
inline void channel_context<Protocol>::write(object_ptr data) {
	_service_p->write_channel(channel_id(), data);
}

template<typename Protocol>
inline void channel_context<Protocol>::close() {
	_service_p->close_channel(channel_id());
}

} // namespace net
} // namespace shan

#endif /* shan_net_channel_context_h */
