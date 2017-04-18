//
//  acceptor_context.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 17..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_acceptor_context_h
#define shan_net_acceptor_context_h

namespace shan {
namespace net {

class acceptor_context : public context_base {
	friend class tcp_server_base;
	friend class tcp_server;
	friend class ssl_server;
public:
	acceptor_context(acceptor&& a) : context_base(a.io_service()), _acceptor(std::move(a)) {}

private:
	void start(uint16_t port, bool reuse_addr, int backlog) {
		if (settable_stat(STARTED)) {
			stat(STARTED);

			_acceptor.open(reuse_addr);
			_acceptor.bind(port);
			_acceptor.listen(backlog);
		}
	}

	void stop() {
		stat(CLOSED);
		_acceptor.close();
	}

	void accept(asio::ip::tcp::socket& peer, const std::function<accept_complete_hander>& accept_handler) {
		if (stat() == STARTED)
			_acceptor.accept(peer, accept_handler);
	}

private:
	acceptor _acceptor;
};

using acceptor_context_ptr = std::shared_ptr<acceptor_context>;

} // namespace net
} // namespace shan

#endif /* shan_net_acceptor_context_h */
