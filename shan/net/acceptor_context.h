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

class acceptor_context : public context {
	friend class server;
public:
	acceptor_context(acceptor_ptr ac_ptr, service* svc_p) : context(ac_ptr->get_io_service(), svc_p), _acceptor_ptr(std::move(ac_ptr)) {}

private:
	void start(uint16_t port, bool reuse_addr, int backlog, const std::function<accept_complete_hander>& accept_handler) {
		if (set_stat_if_possible(STARTED)) {
			_acceptor_ptr->open(reuse_addr);
			_acceptor_ptr->bind(port);
			_acceptor_ptr->listen(backlog);
			_acceptor_ptr->accept(accept_handler);
		}
	}

	void stop() noexcept {
		_acceptor_ptr->close();
		set_stat_if_possible(CLOSED);
	}

	void accept(const std::function<accept_complete_hander>& accept_handler) {
		_acceptor_ptr->accept(accept_handler);
	}

private:
	acceptor_ptr _acceptor_ptr;
};
	
using acceptor_context_ptr = std::unique_ptr<acceptor_context>;

} // namespace net
} // namespace shan

#endif /* shan_net_acceptor_context_h */
