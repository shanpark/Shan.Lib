//
//  channel_base.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_channel_base_h
#define shan_net_channel_base_h

namespace shan {
namespace net {

using connect_complete_handler = void (const asio::error_code& error);
using tcp_connect_complete_handler = void (const asio::error_code& error, asio::ip::tcp::resolver::iterator it);
using udp_connect_complete_handler = void (const asio::error_code& error, asio::ip::udp::resolver::iterator it);
using read_complete_handler = void (const asio::error_code& error, std::size_t bytes_transferred);
using write_complete_handler = void (const asio::error_code& error, std::size_t bytes_transferred);
using shutdown_complete_handler = void (const asio::error_code& error);

template<typename Protocol>
class channel_context;

class generic_channel : public object {
	friend class channel_context<protocol::tcp>;
	friend class channel_context<protocol::udp>;

public:
	virtual void set_receive_buffer_size(int size) = 0;
	virtual void set_send_buffer_size(int size) = 0;

protected:
	virtual std::size_t id() const = 0;

	virtual void open(ip v) = 0;
	virtual void close_gracefully(std::function<shutdown_complete_handler> shudown_handler) noexcept = 0;
	virtual void close_immediately() noexcept = 0;
	virtual void close_without_shutdown() noexcept = 0;
	virtual void cancel_all() noexcept = 0;

	virtual void connect(const ip_port& destination, std::function<connect_complete_handler> connect_handler) = 0;

	virtual void read(util::streambuf_ptr read_sb_ptr, std::function<read_complete_handler> read_handler) noexcept = 0;
	virtual void write_streambuf(util::streambuf_ptr write_sb_ptr, std::function<write_complete_handler> write_handler) = 0;
	virtual void remove_sent_data() = 0;
	virtual bool has_data_to_write() = 0;
	virtual void write_next_streambuf(std::function<write_complete_handler> write_handler) = 0;

	virtual asio::io_service& io_service() = 0;
};

template<typename Protocol>
class channel_base;

template<>
class channel_base<protocol::tcp> : public generic_channel {
public:
	virtual void set_receive_buffer_size(int size) override {
		asio::socket_base::receive_buffer_size option(size);
		socket().set_option(option);
	}

	virtual void set_send_buffer_size(int size) override {
		asio::socket_base::send_buffer_size option(size);
		socket().set_option(option);
	}

	void set_no_delay(bool no_delay) {
		asio::ip::tcp::no_delay option(no_delay);
		socket().set_option(option);
	}

	void set_keep_alive(bool keep_alive) {
		asio::socket_base::keep_alive option(keep_alive);
		socket().set_option(option);
	}

	void set_linger(bool linger, int timeout) {
		asio::socket_base::linger option(linger, timeout);
		socket().set_option(option);
	}

	void set_recv_low_watermark(int recv_low_watermark) {
		asio::socket_base::receive_low_watermark option(recv_low_watermark);
		socket().set_option(option);
	}

	void set_send_low_watermark(int send_low_watermark) {
		asio::socket_base::send_low_watermark option(send_low_watermark);
		socket().set_option(option);
	}

protected:
	virtual asio::ip::tcp::socket& socket() = 0;
};

template<>
class channel_base<protocol::udp> : public generic_channel {
public:
	virtual void set_receive_buffer_size(int size) override {
		asio::socket_base::receive_buffer_size option(size);
		socket().set_option(option);
	}

	virtual void set_send_buffer_size(int size) override {
		asio::socket_base::send_buffer_size option(size);
		socket().set_option(option);
	}

	void set_multicast_loop(bool multicast_loop) {
		asio::ip::multicast::enable_loopback option(multicast_loop);
		socket().set_option(option);
	}

	void set_multicast_ttl(int multicast_ttl) {
		asio::ip::multicast::hops option(multicast_ttl);
		socket().set_option(option);
	}

	void join_group(std::string multicast_address) {
		asio::ip::address ma = asio::ip::address::from_string(multicast_address);
		asio::ip::multicast::join_group option(ma);
		socket().set_option(option);
	}

	void leave_group(std::string multicast_address) {
		asio::ip::address ma = asio::ip::address::from_string(multicast_address);
		asio::ip::multicast::leave_group option(ma);
		socket().set_option(option);
	}

	void set_multicast_if(std::string address) {
		asio::ip::address_v4 local_interface = asio::ip::address_v4::from_string(address);
		asio::ip::multicast::outbound_interface option(local_interface);
		socket().set_option(option);
	}

	void set_unicast_ttl(int unicast_ttl) {
		asio::ip::unicast::hops option(unicast_ttl);
		socket().set_option(option);
	}

	void set_broadcast(bool broadcast) {
		asio::socket_base::broadcast option(broadcast);
		socket().set_option(option);
	}

	void set_dont_route(bool dont_route) {
		asio::socket_base::do_not_route option(dont_route);
		socket().set_option(option);
	}

protected:
	virtual asio::ip::udp::socket& socket() = 0;
};

} // namespace net
} // namespace shan

#include "tcp_channel.h"
#ifdef SHAN_NET_SSL_ENABLE
#include "ssl_channel.h"
#endif
#include "udp_channel.h"

#endif /* shan_net_channel_base_h */
