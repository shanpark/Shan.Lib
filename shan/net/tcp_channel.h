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

template<typename Protocol>
class channel_context;

class tcp_channel : public channel_base<protocol::tcp> {
	friend class channel_context<protocol::tcp>;
	friend class tcp_channel_context;
	friend class tcp_server;
public:
	tcp_channel(asio::ip::tcp::socket&& socket, std::size_t buffer_base_size)
	: _socket(std::move(socket)) {}

	virtual ~tcp_channel() {
		close_without_shutdown();
	}

private:
	virtual std::size_t id() const override {
		return reinterpret_cast<std::size_t>(const_cast<tcp_channel*>(this));
	}

	virtual void open(ip v) override {
		socket().open((v == ip::v6) ? asio::ip::tcp::v6() : asio::ip::tcp::v4());
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

	virtual void connect(const ip_port& destination, std::function<connect_complete_handler> connect_handler) override {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(destination.ip()), destination.port());
		socket().async_connect(ep, connect_handler);
	}

	void connect(asio::ip::tcp::resolver::iterator it, std::function<tcp_connect_complete_handler> connect_handler) {
		asio::async_connect(socket(), it, connect_handler);
	}

	virtual void read(util::streambuf_ptr read_sb_ptr, std::function<read_complete_handler> read_handler) noexcept override {
		socket().async_read_some(asio::buffer(read_sb_ptr->prepare(read_sb_ptr->base_size()), read_sb_ptr->base_size()), read_handler);
	}

	virtual void write_streambuf(util::streambuf_ptr write_sb_ptr, std::function<write_complete_handler> write_handler) override {
		_write_buf_queue.push_back(write_sb_ptr);

		if (_write_buf_queue.size() == 1)
			asio::async_write(socket(), asio::buffer(write_sb_ptr->in_ptr(), write_sb_ptr->in_size()), write_handler);
	}

	virtual void remove_sent_data() override {
		_write_buf_queue.pop_front();
	}

	virtual bool has_data_to_write() override {
		return (!_write_buf_queue.empty());
	}

	virtual void write_next_streambuf(std::function<write_complete_handler> write_handler) override {
		util::streambuf_ptr next_buf_ptr = _write_buf_queue.front();
		asio::async_write(socket(), asio::buffer(next_buf_ptr->in_ptr(), next_buf_ptr->in_size()), write_handler);
	}

	virtual asio::io_service& io_service() override {
		return socket().get_io_service();
	}

	virtual asio::ip::tcp::socket& socket() override {
		return _socket;
	}

private:
	asio::ip::tcp::socket _socket;
	std::deque<util::streambuf_ptr> _write_buf_queue;
};

using tcp_channel_ptr = std::shared_ptr<tcp_channel>;

} // namespace net
} // namespace shan

#endif /* shan_net_tcp_channel_h */
