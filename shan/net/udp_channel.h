//
//  udp_channel.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_udp_channel_h
#define shan_net_udp_channel_h

namespace shan {
namespace net {

template<typename Protocol>
class channel_context;

class udp_channel : public channel_base<protocol::udp> {
	friend class channel_context<protocol::udp>;
	friend class udp_channel_context;
public:
	udp_channel(asio::ip::udp::socket&& socket, std::size_t buffer_base_size)
	: _socket(std::move(socket)) {}

	virtual ~udp_channel() {
		close_without_shutdown();
	}

	virtual std::size_t id() const override {
		return reinterpret_cast<std::size_t>(const_cast<udp_channel*>(this));
	}

private:
	virtual void open(ip v) override {
		socket().open((v == ip::v6) ? asio::ip::udp::v6() : asio::ip::udp::v4());
	}

	virtual void close_gracefully(std::function<shutdown_complete_handler> shutdown_handler) noexcept override {
		close_immediately();
	}

	virtual void close_immediately() noexcept override {
		if (socket().is_open()) {
			asio::error_code ec;
			socket().shutdown(asio::socket_base::shutdown_both, ec);
			socket().close(ec); // Note that, even if the function indicates an error, the underlying descriptor is closed.
		}
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

	// non-virtual. bind() is a udp only method.
	void bind(uint16_t port, ip v = ip::v4) {
		socket().bind(asio::ip::udp::endpoint((v == ip::v6) ? asio::ip::udp::v6() : asio::ip::udp::v4(), port));
	}

	virtual void connect(const ip_port& destination, std::function<connect_complete_handler> connect_handler) override {
		asio::ip::udp::endpoint ep(asio::ip::address::from_string(destination.ip()), destination.port());
		socket().async_connect(ep, connect_handler);
	}

	void connect(asio::ip::udp::resolver::iterator it, std::function<udp_connect_complete_handler> connect_handler) {
		asio::async_connect(socket(), it, connect_handler);
	}

	virtual void read(util::streambuf_ptr read_sb_ptr, std::function<read_complete_handler> read_handler) noexcept override {
		socket().async_receive_from(asio::buffer(read_sb_ptr->prepare(read_sb_ptr->base_size()), read_sb_ptr->base_size()), _sender, read_handler);
	}

	virtual void write_streambuf(util::streambuf_ptr write_sb_ptr, std::function<write_complete_handler> write_handler) override {
		_write_datagram_queue.push_back(__datagram_message(write_sb_ptr, ip_port()));

		if (_write_datagram_queue.size() == 1)
			socket().async_send(asio::buffer(write_sb_ptr->in_ptr(), write_sb_ptr->in_size()), write_handler);
	}

	virtual void remove_sent_data() override {
		_write_datagram_queue.pop_front();
	}

	virtual bool has_data_to_write() override {
		return (!_write_datagram_queue.empty());
	}

	virtual void write_next_streambuf(std::function<write_complete_handler> write_handler) override {
		__datagram_message next_message = _write_datagram_queue.front();
		if (next_message._destination.is_valid())
			socket().async_send_to(asio::buffer(next_message._write_sb_ptr->in_ptr(), next_message._write_sb_ptr->in_size()), next_message._destination.udp_endpoint(), write_handler);
		else
			socket().async_send(asio::buffer(next_message._write_sb_ptr->in_ptr(), next_message._write_sb_ptr->in_size()), write_handler);
	}

	// non-virtual. write_streambuf_to() is a udp only method.
	void write_streambuf_to(util::streambuf_ptr write_sb_ptr, const ip_port& destination, std::function<write_complete_handler> write_handler) {
		_write_datagram_queue.push_back(__datagram_message(write_sb_ptr, destination));

		if (_write_datagram_queue.size() == 1)
			socket().async_send_to(asio::buffer(write_sb_ptr->in_ptr(), write_sb_ptr->in_size()), destination.udp_endpoint(), write_handler);
	}

	virtual asio::io_service& io_service() override {
		return socket().get_io_service();
	}

	asio::ip::udp::socket& socket() {
		return _socket;
	}
	
	asio::ip::udp::endpoint& sender() {
		return _sender;
	}

	struct __datagram_message {
		__datagram_message(util::streambuf_ptr write_sb_ptr, ip_port destination) : _write_sb_ptr(write_sb_ptr), _destination(destination) {}

		util::streambuf_ptr _write_sb_ptr;
		ip_port _destination;
	};

private:
	asio::ip::udp::socket _socket;

	asio::ip::udp::endpoint _sender;
	std::deque<__datagram_message> _write_datagram_queue;
};

using udp_channel_ptr = std::shared_ptr<udp_channel>;

} // namespace net
} // namespace shan

#endif /* shan_net_udp_channel_h */
