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

// ongoing tasks on channel
using channel_task = uint8_t;
enum : uint8_t {
	T_NONE		= 0,

	T_CONNECT	= (1 << 0),
	T_HANDSHAKE = (1 << 1),
	T_READ		= (1 << 2),
	T_WRITE		= (1 << 3),
	T_SHUTDOWN	= (1 << 4),

	T_BUSY		= (T_CONNECT | T_HANDSHAKE | T_READ | T_WRITE)
};

// state of context. The state proceeds only downward, and the context that reached the last state is not reusable.
enum context_stat : uint8_t {
	CREATED = 0,
	OPEN, // all
	STARTED, // acceptor
	BOUND, // udp_channel
	CONNECTED, // tcp_channel, udp_channel
	REQ_CLOSE, // tcp_channel, udp_channel
	DISCONNECTED, // tcp_channel, udp_channel
	CLOSED // all
};

class context_base : public object {
public:
	context_base(asio::io_service& io_service)
	: _done(false), _connected(false), _task_in_progress(T_NONE), _stat(CREATED) {}

	void done(bool done) {
		_done = done;
	}

	bool done() {
		return _done;
	}

protected:
	bool connected() {
		return _connected;
	}

	void connected(bool c) {
		_connected = c;
	}

	void set_task_in_progress(channel_task task) {
		_task_in_progress |= task;
	}

	void clear_task_in_progress(channel_task task) {
		_task_in_progress &= (~task);
	}

	bool is_task_in_progress(channel_task task) {
		return (_task_in_progress | task);
	}

	bool is_channel_busy() {
		return (_task_in_progress & T_BUSY);
	}

	bool settable_stat(context_stat s) {
		if (_stat < s) // the state can not proceed reverse direction.
			return true;
		return false;
	}

	context_stat stat() {
		return _stat;
	}

	void stat(context_stat s) {
		_stat = s;
	}

protected:
	bool _done;
	bool _connected;
	channel_task _task_in_progress;
	context_stat _stat;
};

} // namespace net
} // namespace shan

#include "acceptor_context.h"
#include "channel_context.h"

#endif /* shan_net_context_base_h */
