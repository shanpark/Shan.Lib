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
	: _socket(std::move(socket))
	, _read_sb_ptr(streambuf_pool::get_object(buffer_base_size))
	, _write_strand(_socket.get_io_service()) {}

	virtual ~udp_channel() {
		if (_socket.is_open())
			_socket.close();
		streambuf_pool::return_object(_read_sb_ptr);
	}

	virtual std::size_t id() const {
		return static_cast<std::size_t>(const_cast<udp_channel*>(this)->_socket.native_handle());
	}

private:
	virtual void open(ip v) {
		_socket.open((v == ip::v6) ? asio::ip::udp::v6() : asio::ip::udp::v4());
	}

	virtual void close() noexcept {
		asio::error_code ec;
		if (_socket.is_open()) {
			_socket.shutdown(asio::socket_base::shutdown_both, ec);
			_socket.close(ec); // Note that, even if the function indicates an error, the underlying descriptor is closed.
		}
	}

	// non-virtual. bind() is a udp only method.
	void bind(uint16_t port, ip v = ip::v4) {
		_socket.bind(asio::ip::udp::endpoint((v == ip::v6) ? asio::ip::udp::v6() : asio::ip::udp::v4(), port));
	}

	virtual void connect(const std::string& address, uint16_t port, std::function<connect_complete_handler> connect_handler) {
		asio::ip::udp::endpoint ep(asio::ip::address::from_string(address), port);
		_socket.async_connect(ep, connect_handler);
	}

	virtual void read(std::function<read_complete_handler> read_handler) noexcept {
		_socket.async_receive_from(asio::buffer(_read_sb_ptr->prepare(_read_sb_ptr->base_size()), _read_sb_ptr->base_size()), _sender,
								   std::bind(&udp_channel::read_complete, this, std::placeholders::_1, std::placeholders::_2, read_handler));
	}

	virtual void write_streambuf(util::streambuf_ptr write_sb_ptr, std::function<write_complete_handler> write_handler) {
		_write_strand.post([this, write_sb_ptr, write_handler]() {
			_write_datagram_queue.push_back(__datagram_message(write_sb_ptr, ip_port()));

			if (_write_datagram_queue.size() == 1)
				_socket.async_send(asio::buffer(write_sb_ptr->in_ptr(), write_sb_ptr->in_size()),
								   std::bind(&udp_channel::write_complete, this, std::placeholders::_1, std::placeholders::_2, write_sb_ptr, write_handler));
		});
	}

	// non-virtual. write_streambuf_to() is a udp only method.
	void write_streambuf_to(util::streambuf_ptr write_sb_ptr, const ip_port& destination, std::function<write_complete_handler> write_handler) {
		_write_strand.post([this, write_sb_ptr, destination, write_handler]() {
			_write_datagram_queue.push_back(__datagram_message(write_sb_ptr, destination));

			if (_write_datagram_queue.size() == 1)
				_socket.async_send_to(asio::buffer(write_sb_ptr->in_ptr(), write_sb_ptr->in_size()),
									  destination.udp_endpoint(),
									  std::bind(&udp_channel::write_complete, this, std::placeholders::_1, std::placeholders::_2, write_sb_ptr, write_handler));
		});
	}

	virtual asio::io_service& io_service() {
		return _socket.get_io_service();
	}
	
	asio::ip::udp::endpoint& sender() {
		return _sender;
	}

	void read_complete(const asio::error_code& error, std::size_t bytes_transferred, std::function<read_complete_handler> read_handler) {
		if (!error)
			_read_sb_ptr->commit(bytes_transferred);

		auto read_data = _read_sb_ptr; // send read buffer itself to higher layer to remove data copy
		_read_sb_ptr = streambuf_pool::get_object(_read_sb_ptr->base_size()); // set a new streambuf as a read buffer.

		read_handler(error, read_data); // read_data should be returned to the pool in read_handler().
	}

	void write_complete(const asio::error_code& error, std::size_t bytes_transferred, util::streambuf_ptr write_sb_ptr, std::function<write_complete_handler> write_handler) {
		_write_strand.post([this, error, bytes_transferred, write_sb_ptr, write_handler]() {
			assert(_write_datagram_queue.front()._write_sb_ptr == write_sb_ptr);
			_write_datagram_queue.pop_front();

			write_handler(error, bytes_transferred, write_sb_ptr);

			if (!_write_datagram_queue.empty()) {
				__datagram_message next_message = _write_datagram_queue.front();
				if (next_message._destination.is_valid())
					_socket.async_send_to(asio::buffer(next_message._write_sb_ptr->in_ptr(), next_message._write_sb_ptr->in_size()),
										  next_message._destination.udp_endpoint(),
										  std::bind(&udp_channel::write_complete, this, std::placeholders::_1, std::placeholders::_2, write_sb_ptr, write_handler));
				else
					_socket.async_send(asio::buffer(next_message._write_sb_ptr->in_ptr(), next_message._write_sb_ptr->in_size()),
									   std::bind(&udp_channel::write_complete, this, std::placeholders::_1, std::placeholders::_2, write_sb_ptr, write_handler));
			}
		});
	}

	struct __datagram_message {
		__datagram_message(util::streambuf_ptr write_sb_ptr, ip_port destination) : _write_sb_ptr(write_sb_ptr), _destination(destination) {}

		util::streambuf_ptr _write_sb_ptr;
		ip_port _destination;
	};

private:
	asio::ip::udp::socket _socket;

	asio::ip::udp::endpoint _sender;
	util::streambuf_ptr _read_sb_ptr;
	std::deque<__datagram_message> _write_datagram_queue;
	asio::io_service::strand _write_strand;
};

using udp_channel_ptr = std::unique_ptr<udp_channel>;

} // namespace net
} // namespace shan

#endif /* shan_net_udp_channel_h */
