//
//  tcp_channel_base.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 20..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_channel_base_h
#define shan_net_tcp_channel_base_h

namespace shan {
namespace net {

class tcp_channel_base : public channel_base<protocol::tcp> {
public:

protected:
	virtual void open(ip v) override {
		socket().open((v == ip::v6) ? asio::ip::tcp::v6() : asio::ip::tcp::v4());
	}

	virtual void close_without_shutdown() noexcept override {
		if (socket().is_open()) {
			asio::error_code ec;
			socket().close(ec); // Note that, even if the function indicates an error, the underlying descriptor is closed.
		}
	}

	virtual void cancel_all() noexcept override {
		socket().cancel();
	}

	virtual void connect(const ip_port& destination, std::function<connect_complete_handler> connect_handler) override {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(destination.ip()), destination.port());
		socket().async_connect(ep, connect_handler);
	}

	void connect(asio::ip::tcp::resolver::iterator it, std::function<tcp_connect_complete_handler> connect_handler) {
		asio::async_connect(socket(), it, connect_handler);
	}

	virtual void remove_sent_data() override {
		_write_buf_queue.pop_front();
	}

	virtual bool has_data_to_write() override {
		return (!_write_buf_queue.empty());
	}

protected:
	std::deque<util::streambuf_ptr> _write_buf_queue;
};

} // namespace net
} // namespace shan

#include <shan/net/tcp_channel.h>
#ifdef SHAN_NET_SSL_ENABLE
#include <shan/net/ssl_channel.h>
#endif

#endif /* shan_net_tcp_channel_base_h */
