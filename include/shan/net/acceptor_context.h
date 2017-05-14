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

class tcp_server_base;

class acceptor_context : public context_base {
	friend class tcp_server_base;
public:
	acceptor_context(acceptor&& a) : context_base(), _acceptor(std::move(a)) {}

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

	void call_channel_accepted(tcp_server_base* service_p, const asio::ip::tcp::endpoint& peer_endpoint);
	void call_exception_caught(tcp_server_base* service_p, const acceptor_error& e);

private:
	acceptor _acceptor;
};

using acceptor_context_ptr = std::shared_ptr<acceptor_context>;

} // namespace net
} // namespace shan

#include <shan/net/tcp_server_base.h>

namespace shan {
namespace net {

inline void acceptor_context::call_channel_accepted(tcp_server_base* service_p, const asio::ip::tcp::endpoint& peer_endpoint) {
	done(false); // reset context to 'not done'.
	// <-- inbound
	auto begin = service_p->acceptor_handlers().begin();
	auto end = service_p->acceptor_handlers().end();
	try {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->channel_accepted(this, peer_endpoint.address().to_string(), peer_endpoint.port());
	} catch (const std::exception& e) {
		call_exception_caught(service_p, acceptor_error(std::string("An exception has thrown in channel_accepted handler. (") + e.what() + ")"));
	}
}

inline void acceptor_context::call_exception_caught(tcp_server_base* service_p, const acceptor_error& e) {
	done(false); // reset context to 'not done'.
	// <-- inbound
	auto begin = service_p->acceptor_handlers().begin();
	auto end = service_p->acceptor_handlers().end();
	try {
		for (auto it = begin ; !done() && (it != end) ; it++)
			(*it)->exception_caught(this, e);
	} catch (const std::exception& e2) {
		call_exception_caught(service_p, acceptor_error(std::string("An exception has thrown in exception_caught handler. (") + e2.what() + ")")); // can cause infinite exception...
	}
}

} // namespace net
} // namespace shan

#endif /* shan_net_acceptor_context_h */
