//
//  shan_net_test.cpp
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 29..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <ctime>
#include "net/net.h"
#include "util/pool.h"

using namespace std;
using namespace shan::net;

class unix_time : public shan::object {
public:
	unix_time(std::time_t time) { _time = time; }
	std::time_t get_time() { return _time; }

public:
	std::time_t _time;
};

extern std::mutex _mutex;

udp_service* userv_p;
udp_service* ucli_p;

class channel_coder_u : public udp_channel_handler {
	virtual void channel_read_from(udp_channel_context* ctx, shan::object_ptr& data, const shan::net::ip_port& from) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto sb_ptr = static_cast<shan::util::streambuf*>(data.get());
		if (sb_ptr->in_size() < sizeof(std::time_t)) { // not enough data
			ctx->done(true);
			return;
		}

		std::time_t time;
		if (sizeof(std::time_t) == 4)
			sb_ptr->read_int32(reinterpret_cast<int32_t*>(&time));
		else
			sb_ptr->read_int64(reinterpret_cast<int64_t*>(&time));

		data = std::make_shared<unix_time>(time);

	}

	virtual void channel_write(udp_channel_context* ctx, shan::object_ptr& data) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto time_ptr = static_cast<unix_time*>(data.get());

		auto sb_ptr = std::make_shared<shan::util::streambuf>(sizeof(std::time_t));
		if (sizeof(std::time_t) == 4)
			sb_ptr->write_int32(time_ptr->get_time());
		else
			sb_ptr->write_int64(time_ptr->get_time());

		data = sb_ptr; // return new converted object.
	}
};

class serv_ch_handler_u : public udp_channel_handler {
public:
	virtual ~serv_ch_handler_u() {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << ">>>> serv_ch_handler destroyed!!!!" << endl;
	};

	virtual void user_event(context* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "user_event() called" << endl;
	}

	virtual void exception_caught(context* ctx, const std::exception& e) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "exception_caught() - " << e.what() << endl;
	}

	virtual void channel_bound(udp_channel_context* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_bound(" << ctx->channel_id() << ") called" << endl;
	}

	virtual void channel_connected(udp_channel_context* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_connected(" << ctx->channel_id() << ") called" << endl;
	}

	virtual void channel_read_from(udp_channel_context* ctx, shan::object_ptr& data, const ip_port& from) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_read_from(" << from.ip() << ":" << from.port() << ")" << endl;
		auto time = static_cast<unix_time*>(data.get());
		cout << "time:" << time->get_time() << endl;

//		userv_p->write_channel_to(ctx->channel_id(), from, data);
		ctx->write_to(from, data);
	}

	virtual void channel_write(udp_channel_context* ctx, shan::object_ptr& data) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto sb_ptr = static_cast<shan::util::streambuf*>(data.get());
		cout << "serv_ch_handler::" << "channel_write() - " << sb_ptr->in_size() << endl;
		ctx->done(true);
	}

	virtual void channel_written(udp_channel_context* ctx, std::size_t bytes_transferred, shan::util::streambuf_ptr sb_ptr) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_written() - " << bytes_transferred << endl;

		ctx->close();
		userv_p->stop();
	}

	virtual void channel_disconnected(udp_channel_context* ctx) override {
		static int c = 0;
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_disconnected() called:" << ++c << endl;

//		userv_p->stop(); // stop이 호출되면 더 이상의 이벤트는 발생하지 않는다.
	}
};

class cli_ch_handler_u : public udp_channel_handler {
public:
	~cli_ch_handler_u() {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << ">>>> cli_ch_handler destroyed" << endl;
	};

	virtual void user_event(context* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "user_event() called" << endl;
	}

	virtual void exception_caught(context* ctx, const std::exception& e) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "exception_caught() - " << e.what() << endl;
	}

	virtual void channel_bound(udp_channel_context* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_bound(" << ctx->channel_id() << ") called" << endl;
	}

	virtual void channel_connected(udp_channel_context* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_connected(" << ctx->channel_id() << ") called" << endl;
		auto data = std::make_shared<unix_time>(3000);
		ctx->write(data);
	}

	virtual void channel_read_from(udp_channel_context* ctx, shan::object_ptr& data, const ip_port& from) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_read_from() called" << endl;
		auto time = static_cast<unix_time*>(data.get());
		cout << "time:" << time->get_time() << endl;
		ctx->close();
	}

	virtual void channel_write(udp_channel_context* ctx, shan::object_ptr& data) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto sb_ptr = static_cast<shan::util::streambuf*>(data.get());
		cout << "cli_ch_handler::" << "channel_write() - " << sb_ptr->in_size() << endl;
	}

	virtual void channel_written(udp_channel_context* ctx, std::size_t bytes_transferred, shan::util::streambuf_ptr sb_ptr) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_written() called" << endl;
	}

	virtual void channel_disconnected(udp_channel_context* ctx) override {
		static int c = 0;
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_disconnected() called:" << ++c << endl;

		ucli_p->stop(); // stop()호출 뒤에 발생되는 이벤트는 핸들러 호출이 되지 않는다.
	}
};

void shan_net_udp_test() {
	shan::net::udp_service serv(1);
	serv.add_channel_handler(new channel_coder_u()); //
	serv.add_channel_handler(new serv_ch_handler_u()); //
	serv.start();
	userv_p = &serv;

	serv.bind_connect(4747, ip_port()); // local binding. no connect.
	{
	shan::net::udp_service cli(1);
	cli.add_channel_handler(new channel_coder_u()); //
	cli.add_channel_handler(new cli_ch_handler_u()); // 이 핸들러는 cli가 destroy될 때 같이 해제된다.
	cli.start();
	ucli_p = &cli;
	cli.bind_connect(shan::net::ANY, "127.0.0.1", 4747); // local binding and connect.

	cli.wait_stop();
	}
	serv.wait_stop();
}
