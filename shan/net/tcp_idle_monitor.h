//
//  tcp_idle_monitor.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 21..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_handler_tcp_idle_monitor_h
#define shan_net_handler_tcp_idle_monitor_h

namespace shan {
namespace net {

template<typename Protocol>
class service_base;

class tcp_idle_monitor : public channel_handler<protocol::tcp> {
public:
	tcp_idle_monitor(int64_t id, int64_t expire_in_millis, object_ptr param_ptr)
	: _id(id), _expire_time(expire_in_millis), _param_ptr(param_ptr) {
		if (_expire_time <= 0)
			throw std::invalid_argument("invalid expiration time.");
	}

	virtual void channel_connected(shan::net::tcp_channel_context_base* ctx) override {
		_service_p = ctx->service_p();
		_monitors.emplace(ctx->channel_id(), std::unique_ptr<_monitor>(new _monitor(ctx->io_service(), _expire_time, ctx->strand().wrap(std::bind(&tcp_idle_monitor::timeout, this, std::placeholders::_1, ctx->channel_id())))));
	}

	virtual void channel_read(shan::net::tcp_channel_context_base* ctx, shan::object_ptr& data) override {
		try {
			auto& mon_ptr = _monitors.at(ctx->channel_id());
			mon_ptr->reset_timer(_expire_time, ctx->strand().wrap(std::bind(&tcp_idle_monitor::timeout, this, std::placeholders::_1, ctx->channel_id())));
		} catch (const std::exception&) {
			// no such channel exists. nothing happens.
		}
	}

	virtual void channel_write(shan::net::tcp_channel_context_base* ctx, shan::object_ptr& data) override {
		try {
			auto& mon_ptr = _monitors.at(ctx->channel_id());
			mon_ptr->reset_timer(_expire_time, ctx->strand().wrap(std::bind(&tcp_idle_monitor::timeout, this, std::placeholders::_1, ctx->channel_id())));
		} catch (const std::exception&) {
			// no such channel exists. nothing happens.
		}
	}

	virtual void channel_disconnected(shan::net::tcp_channel_context_base* ctx) override {
		try {
			_monitors.erase(ctx->channel_id());
		} catch (const std::exception&) {
			// no such channel exists. nothing happens.
		}
	}

private:
	void timeout(const asio::error_code& error, int64_t channel_id) {
		if (error)
			return; // cancelled. do nothing.

		try {
			// fire user_event
			_service_p->fire_user_event_channel(channel_id, _id, _param_ptr);

			// timer reset
			auto& mon_ptr = _monitors.at(channel_id);
			channel_context<protocol::tcp>* ctx = _service_p->channel_context_of(channel_id);
			mon_ptr->reset_timer(_expire_time, ctx->strand().wrap(std::bind(&tcp_idle_monitor::timeout, this, std::placeholders::_1, channel_id)));
		} catch (const std::exception&) {
			// no such channel exists. nothing happens.
		}
	}

	class _monitor {
	public:
		_monitor(asio::io_service& io_service, int64_t expire_time, std::function<void(const asio::error_code& error)> timeout_handler)
		: _timer(io_service) {
			reset_timer(expire_time, timeout_handler);
		}

		~_monitor() {
			_timer.cancel();
		}

		void reset_timer(int64_t expire_time, std::function<void(const asio::error_code& error)> timeout_handler) {
			asio::error_code ec;
			_timer.expires_from_now(std::chrono::milliseconds(expire_time), ec);
			_timer.async_wait(timeout_handler);
		}

	public:
		asio::steady_timer _timer; // steady_timer is not copiable, movable. timer classes will be movable from asio 1.11.0.
								   // so unique_ptr<_monitor> is used.
	};

private:
	int64_t _id;
	int64_t _expire_time;
	object_ptr _param_ptr;
	service_base<protocol::tcp>* _service_p;
	std::unordered_map<std::size_t, std::unique_ptr<_monitor>> _monitors;
};

} // namespace net
} // namespace shan

#endif /* shan_net_handler_tcp_idle_monitor_h */
