//
//  acceptor.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_acceptor_h
#define shan_net_acceptor_h

namespace shan {
namespace net {

using accept_complete_hander = void(const asio::error_code& error, const asio::ip::tcp::endpoint& peer_endpoint);

class acceptor : public object {
	friend class acceptor_context;
public:
	acceptor(asio::io_service& io_service, ip v)
	: _ipv((v == ip::v6) ? asio::ip::tcp::v6() : asio::ip::tcp::v4()), _acceptor(io_service) {}

private:
	void open(bool reuse_addr = true) {
		_acceptor.open(_ipv);
		if (reuse_addr)
			_acceptor.set_option(asio::socket_base::reuse_address(true));
	}

	void close() noexcept {
		asio::error_code ec;
		if (_acceptor.is_open())
			_acceptor.close(ec);
	}

	void bind(uint16_t port) {
		_acceptor.bind(asio::ip::tcp::endpoint(_ipv, port));
	}

	void listen(int listen_backlog) {
		_acceptor.listen(listen_backlog);
	}

	void accept(asio::ip::tcp::socket& peer, const std::function<accept_complete_hander>& accept_handler) {
		_acceptor.async_accept(peer, _peer_endpoint, std::bind(&acceptor::accept_complete, this, std::placeholders::_1, accept_handler));
	}

	asio::io_service& io_service() {
		return _acceptor.get_io_service();
	}

	void accept_complete(const asio::error_code& error, const std::function<accept_complete_hander>& accept_handler) {
		accept_handler(error, _peer_endpoint);
	}

private:
	asio::ip::tcp _ipv;
	asio::ip::tcp::acceptor _acceptor;
	asio::ip::tcp::endpoint _peer_endpoint;
};

using acceptor_ptr = std::unique_ptr<acceptor>;

} // namespace net
} // namespace shan

#endif /* shan_net_acceptor_h */
