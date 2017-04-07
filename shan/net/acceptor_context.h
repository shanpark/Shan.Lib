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
	acceptor_context(acceptor_ptr ac_ptr) : context_base(ac_ptr->io_service()), _acceptor_ptr(std::move(ac_ptr)) {}

private:
	void start(uint16_t port, bool reuse_addr, int backlog) {
		if (settable_stat(STARTED)) {
			_acceptor_ptr->open(reuse_addr);
			_acceptor_ptr->bind(port);
			_acceptor_ptr->listen(backlog);
			stat(STARTED);
		}
	}

	void stop() noexcept {
		_acceptor_ptr->close();
		stat(CLOSED);
	}

	void accept(asio::ip::tcp::socket& peer, const std::function<accept_complete_hander>& accept_handler) {
		if (stat() == STARTED)
			_acceptor_ptr->accept(peer, accept_handler);
	}

private:
	acceptor_ptr _acceptor_ptr;
};
	
using acceptor_context_ptr = std::unique_ptr<acceptor_context>;

} // namespace net
} // namespace shan

#endif /* shan_net_acceptor_context_h */
