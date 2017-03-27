//
//  tcp_channel.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 15..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_channel_h
#define shan_net_tcp_channel_h

namespace shan {
namespace net {

class tcp_channel : public channel {
	friend class channel_context;
public:
	tcp_channel(asio::ip::tcp::socket&& socket, std::size_t buffer_base_size)
	: channel(), _socket(std::move(socket)), _streambuf_ptr(std::make_shared<util::streambuf>(buffer_base_size)), _write_strand(_socket.get_io_service()) {}

	virtual std::size_t id() const { return static_cast<std::size_t>(const_cast<tcp_channel*>(this)->_socket.native_handle()); }

private:
	virtual void open(ip v) { _socket.open((v == ip::v6) ? asio::ip::tcp::v6() : asio::ip::tcp::v4()); }

	virtual void close() noexcept {
		asio::error_code ec;
		if (_socket.is_open()) {
			_socket.shutdown(asio::socket_base::shutdown_both, ec);
			_socket.close(ec); // Note that, even if the function indicates an error, the underlying descriptor is closed.
		}
	}

	void connect(asio::ip::tcp::endpoint ep, std::function<connect_complete_handler> connect_handler) {
		_socket.async_connect(ep, connect_handler);
	}

	virtual void read(std::function<read_complete_handler> read_handler) noexcept {
		_socket.async_read_some(asio::buffer(_streambuf_ptr->prepare(_streambuf_ptr->base_size()), _streambuf_ptr->base_size()),
								std::bind(&tcp_channel::read_complete, this, std::placeholders::_1, std::placeholders::_2, read_handler));
	}

	virtual void write_stream(util::streambuf_ptr write_buf_ptr, std::function<write_complete_handler> write_handler) {
		_write_strand.post([this, write_buf_ptr, write_handler]() {
			_write_buf_queue.push_back(write_buf_ptr);

			if (_write_buf_queue.size() == 1)
				asio::async_write(_socket, asio::buffer(write_buf_ptr->in_ptr(), write_buf_ptr->in_size()),
								  std::bind(&tcp_channel::write_complete, this, std::placeholders::_1, std::placeholders::_2, write_buf_ptr, write_handler));
		});
	}

	virtual asio::io_service& get_io_service() { return _socket.get_io_service(); }

	void read_complete(const asio::error_code& error, std::size_t bytes_transferred, std::function<read_complete_handler> read_handler) {
		if (!error)
			_streambuf_ptr->commit(bytes_transferred);

		read_handler(error, _streambuf_ptr); // _streambuf_ptr's data is copied in the handler.
	}

	void write_complete(const asio::error_code& error, std::size_t bytes_transferred, util::streambuf_ptr write_buf_ptr, std::function<write_complete_handler> write_handler) {
		_write_strand.post([this, error, bytes_transferred, write_buf_ptr, write_handler]() {
			assert(_write_buf_queue.front() == write_buf_ptr);
			_write_buf_queue.pop_front();

			write_handler(error, bytes_transferred, write_buf_ptr);

			if (!_write_buf_queue.empty()) {
				util::streambuf_ptr next_buf_ptr = _write_buf_queue.front();
				asio::async_write(_socket, asio::buffer(next_buf_ptr->in_ptr(), next_buf_ptr->in_size()), std::bind(&tcp_channel::write_complete, this, std::placeholders::_1, std::placeholders::_2, next_buf_ptr, write_handler));
			}
		});
	}

private:
	asio::ip::tcp::socket _socket;

	util::streambuf_ptr _streambuf_ptr;
	std::deque<util::streambuf_ptr> _write_buf_queue;
	asio::io_service::strand _write_strand;
};

} // namespace net
} // namespace shan

#endif /* shan_net_tcp_channel_h */
