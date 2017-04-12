//
//  context_base.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_context_base_h
#define shan_net_context_base_h

namespace shan {
namespace net {

class context_base : public object {
public:
	context_base(asio::io_service& io_service)
	: _done(false), _connected(false), _stat(CREATED) {}

	void done(bool done) {
		_done = done;
	}

	bool done() {
		return _done;
	}

protected:
	// The state proceeds only downward, and the context that reached the last state is not reusable.
	enum : uint8_t {
		CREATED = 0,
		OPEN, // all
		STARTED, // acceptor
		BOUND, // udp_channel
		CONNECTED, // tcp_channel, udp_channel
		DISCONNECTED, // tcp_channel, udp_channel
		CLOSED // all
	};

	bool settable_stat(uint8_t s) {
		if (_stat < s) // the state can not proceed reverse direction.
			return true;
		return false;
	}

	bool connected() {
		return _connected;
	}

	void connected(bool c) {
		_connected = c;
	}

	uint8_t stat() {
		return _stat;
	}

	void stat(uint8_t s) {
		_stat = s;
	}

protected:
	bool _done;
	bool _connected;
	uint8_t _stat;
};

} // namespace net
} // namespace shan

#include "acceptor_context.h"
#include "channel_context.h"

#endif /* shan_net_context_base_h */
