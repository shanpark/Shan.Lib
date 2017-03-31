//
//  tcp_service.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_tcp_service_h
#define shan_net_tcp_service_h

namespace shan {
namespace net {

class tcp_service : public service {
public:
	tcp_service(std::size_t worker_count = 2, std::size_t buffer_base_size = default_buffer_base_size)
	: service(protocol::tcp, worker_count, buffer_base_size) {}

protected:
	virtual void fire_channel_read(channel_context_ptr ch_ctx_ptr, util::streambuf_ptr& sb_ptr) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr, sb_ptr]() { //
			// copy to context's buffer
			ch_ctx_ptr->read_buf()->append(*sb_ptr);
			streambuf_pool::return_object(sb_ptr); // return for reuse.

			ch_ctx_ptr->done(false); // reset context to 'not done'.
			do {
				auto before_size = ch_ctx_ptr->read_buf()->in_size();

				auto object_ptr = std::static_pointer_cast<object>(ch_ctx_ptr->read_buf());
				// <-- inbound
				auto begin = channel_handlers().begin();
				auto end = channel_handlers().end();
				try {
					for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
						(*it)->channel_read(static_cast<tcp_channel_context*>(ch_ctx_ptr.get()), object_ptr);
				} catch (const std::exception& e) {
					fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_read handler. (") + e.what() + ")"));
				}

				if (ch_ctx_ptr->stat() > channel_context::CONNECTED) // if close() called in handler...
					return;

				if (ch_ctx_ptr->read_buf()->in_size() == before_size) // no data used.
					break;
			} while (ch_ctx_ptr->read_buf()->in_size() > 0); // available data remains.

			if (ch_ctx_ptr->read_buf()->in_size() == 0)
				fire_channel_rdbuf_empty(ch_ctx_ptr);
		});
	}

	void fire_channel_rdbuf_empty(channel_context_ptr ch_ctx_ptr) {
		ch_ctx_ptr->strand().post([this, ch_ctx_ptr]() {
			ch_ctx_ptr->done(false); // reset context to 'not done'.
			// <-- inbound
			auto begin = channel_handlers().begin();
			auto end = channel_handlers().end();
			try {
				for (auto it = begin ; !(ch_ctx_ptr->done()) && (it != end) ; it++)
					(*it)->channel_rdbuf_empty(static_cast<tcp_channel_context*>(ch_ctx_ptr.get()));
			} catch (const std::exception& e) {
				fire_channel_exception_caught(ch_ctx_ptr, channel_error(std::string("An exception has thrown in channel_rdbuf_empty handler. (") + e.what() + ")"));
			}
		});
	}
};

} // namespace net
} // namespace shan

#include "server.h"
#include "client.h"

#endif /* shan_net_tcp_service_h */
