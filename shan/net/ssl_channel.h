//
//  ssl_channel.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 4..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_ssl_channel_h
#define shan_net_ssl_channel_h

namespace shan {
namespace net {

using handshake_complete_hander = void (const asio::error_code& error);

class ssl_channel : public channel_base<protocol::tcp> {
	friend class channel_context<protocol::tcp>;
	friend class ssl_channel_context;
	friend class ssl_server;
public:
	ssl_channel(asio::io_service& io_service, asio::ssl::context& ssl_context, std::size_t buffer_base_size)
	: _socket(io_service, ssl_context)
	, _read_sb_ptr(streambuf_pool::get_object(buffer_base_size))
	, _write_strand(_socket.get_io_service()) {}
	
	virtual ~ssl_channel() {
		if (_socket.lowest_layer().is_open())
			close();
		streambuf_pool::return_object(_read_sb_ptr);
	}

	virtual std::size_t id() const {
		return static_cast<std::size_t>(const_cast<ssl_channel*>(this)->_socket.lowest_layer().native_handle());
	}

private:
	virtual void open(ip v) {
		_socket.lowest_layer().open((v == ip::v6) ? asio::ip::tcp::v6() : asio::ip::tcp::v4());
	}

	virtual void close() noexcept {
		asio::error_code ec;
		if (_socket.lowest_layer().is_open()) {
			_socket.shutdown(ec);
			_socket.lowest_layer().close(ec); // Note that, even if the function indicates an error, the underlying descriptor is closed.
		}
	}

	virtual void connect(const std::string& address, uint16_t port, std::function<connect_complete_handler> connect_handler) {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(address), port);
		_socket.lowest_layer().async_connect(ep, connect_handler);
	}

	virtual void read(std::function<read_complete_handler> read_handler) noexcept {
		_socket.async_read_some(asio::buffer(_read_sb_ptr->prepare(_read_sb_ptr->base_size()), _read_sb_ptr->base_size()),
								std::bind(&ssl_channel::read_complete, this, std::placeholders::_1, std::placeholders::_2, read_handler));
	}

	virtual void write_streambuf(util::streambuf_ptr write_sb_ptr, std::function<write_complete_handler> write_handler) {
		_write_strand.post([this, write_sb_ptr, write_handler]() {
			_write_buf_queue.push_back(write_sb_ptr);

			if (_write_buf_queue.size() == 1)
				asio::async_write(_socket, asio::buffer(write_sb_ptr->in_ptr(), write_sb_ptr->in_size()),
								  std::bind(&ssl_channel::write_complete, this, std::placeholders::_1, std::placeholders::_2, write_sb_ptr, write_handler));
		});
	}

	virtual asio::io_service& io_service() {
		return _socket.get_io_service();
	}

	void handshake(asio::ssl::stream_base::handshake_type type, const std::function<handshake_complete_hander>& handshake_handler) {
		_socket.async_handshake(type, handshake_handler);
	}

	asio::ip::tcp::socket& socket() {
		return static_cast<asio::ip::tcp::socket&>(_socket.lowest_layer());
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
			assert(_write_buf_queue.front() == write_sb_ptr);
			_write_buf_queue.pop_front();

			write_handler(error, bytes_transferred, write_sb_ptr);

			if (!_write_buf_queue.empty()) {
				util::streambuf_ptr next_buf_ptr = _write_buf_queue.front();
				asio::async_write(_socket, asio::buffer(next_buf_ptr->in_ptr(), next_buf_ptr->in_size()), std::bind(&ssl_channel::write_complete, this, std::placeholders::_1, std::placeholders::_2, next_buf_ptr, write_handler));
			}
		});
	}

private:
	asio::ssl::stream<asio::ip::tcp::socket> _socket;

	util::streambuf_ptr _read_sb_ptr;
	std::deque<util::streambuf_ptr> _write_buf_queue;
	asio::io_service::strand _write_strand;
};

using ssl_channel_ptr = std::unique_ptr<ssl_channel>;

} // namespace net
} // namespace shan

#endif /* shan_net_ssl_channel_h */
