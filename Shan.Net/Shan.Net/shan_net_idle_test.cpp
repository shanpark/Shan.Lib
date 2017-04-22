//
//  shan_net_ssl_test.cpp
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 5..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <ctime>
#include "net/net.h"
#include "util/pool.h"

#define IDLE_EVENT	1

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

tcp_server* iserv_p;
tcp_client* icli_p;

class acpt_handler_i : public acceptor_handler {
	virtual void user_event(acceptor_context* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "acpt_handler::" << "user_event() called" << endl;
	}

	virtual void exception_caught(acceptor_context* ctx, const std::exception& e) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "acpt_handler::" << "exception_caught() - " << e.what() << endl;
	}

	virtual void channel_accepted(acceptor_context* ctx, const std::string& address, uint16_t port) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "acpt_handler::" << "channel_accepted() - " << "connection from '" << address << ":" << port << "'" << endl;
	}
};

class channel_coder_i : public tcp_channel_handler {
	virtual void channel_read(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
//		std::lock_guard<std::mutex> _lock(_mutex);
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

	virtual void channel_write(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
//		std::lock_guard<std::mutex> _lock(_mutex);
		auto time_ptr = static_cast<unix_time*>(data.get());

		auto sb_ptr = std::make_shared<shan::util::streambuf>(sizeof(std::time_t));
		if (sizeof(std::time_t) == 4)
			sb_ptr->write_int32(static_cast<int32_t>(time_ptr->get_time()));
		else
			sb_ptr->write_int64(static_cast<int32_t>(time_ptr->get_time()));

		data = sb_ptr; // return new converted object.
	}
};

class serv_ch_handler_i : public tcp_channel_handler {
public:
	virtual ~serv_ch_handler_i() {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << ">>>> serv_ch_handler destroyed!!!!" << endl;
	};

	virtual void user_event(tcp_channel_context_base* ctx, int64_t id, shan::object_ptr data_ptr) override {
		static int c = 0;
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "serv_ch_handler::" << "user_event() called:" << ++c << endl;
			
			if (id == IDLE_EVENT) {
				if (c == 1) {
					auto data = std::make_shared<unix_time>(3000);
					ctx->write(data);
				}
				else { // (c == 2)
					ctx->close();
				}
			}
		}
	}

	virtual void exception_caught(tcp_channel_context_base* ctx, const std::exception& e) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "exception_caught() - " << e.what() << endl;
	}

	virtual void channel_created(shan::net::tcp_channel_context_base* ctx, channel_base<protocol::tcp>* channel) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_created(" << ctx->channel_id() << ") called" << endl;
	}

	virtual void channel_connected(tcp_channel_context_base* ctx) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "serv_ch_handler::" << "channel_connected(" << ctx->channel_id() << ") called" << endl;
		}
	}

	virtual void channel_read(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_read()" << endl;
	}

	virtual void channel_rdbuf_empty(tcp_channel_context_base* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_rdbuf_empty() called" << endl;
	}

	virtual void channel_write(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			auto sb_ptr = static_cast<shan::util::streambuf*>(data.get());
			cout << "serv_ch_handler::" << "channel_write() - " << sb_ptr->in_size() << endl;
		}
	}

	virtual void channel_written(tcp_channel_context_base* ctx, std::size_t bytes_transferred) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "serv_ch_handler::" << "channel_written() - " << bytes_transferred << endl;
		}
	}

	virtual void channel_disconnected(tcp_channel_context_base* ctx) override {
		static int c = 0;
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "serv_ch_handler::" << "channel_disconnected() called:" << ++c << endl;
		}

		iserv_p->request_stop();
	}
};

class cli_ch_handler_i : public tcp_channel_handler {
public:
	virtual ~cli_ch_handler_i() {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << ">>>> cli_ch_handler destroyed" << endl;
	};

	virtual void user_event(tcp_channel_context_base* ctx, int64_t id, shan::object_ptr data_ptr) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "user_event() called" << endl;
	}

	virtual void exception_caught(tcp_channel_context_base* ctx, const std::exception& e) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "exception_caught() - " << e.what() << endl;
	}

	virtual void channel_created(shan::net::tcp_channel_context_base* ctx, channel_base<protocol::tcp>* channel) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_created(" << ctx->channel_id() << ") called" << endl;
	}

	virtual void channel_connected(tcp_channel_context_base* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_connected(" << ctx->channel_id() << ") called" << endl;
	}

	virtual void channel_read(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "cli_ch_handler::" << "channel_read() called" << endl;
			auto time = static_cast<unix_time*>(data.get());
			cout << "time:" << time->get_time() << endl;
		}
	}

	virtual void channel_rdbuf_empty(tcp_channel_context_base* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_rdbuf_empty() called" << endl;
	}

	virtual void channel_write(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto sb_ptr = static_cast<shan::util::streambuf*>(data.get());
		cout << "cli_ch_handler::" << "channel_write() - " << sb_ptr->in_size() << endl;
	}

	virtual void channel_written(tcp_channel_context_base* ctx, std::size_t bytes_transferred) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_written() called" << endl;
	}

	virtual void channel_disconnected(tcp_channel_context_base* ctx) override {
		static int c = 0;
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "cli_ch_handler::" << "channel_disconnected() called:" << ++c << endl;
		}
		icli_p->request_stop(); // stop()호출 뒤에 발생되는 이벤트는 핸들러 호출이 되지 않는다.
	}
};

void shan_net_idle_test() {
	shan::net::tcp_server serv;
	serv.add_acceptor_handler(new acpt_handler_i()); // 이 핸들러는 serv가 destroy될 때 같이 해제된다. 걱정마라..
	serv.add_channel_handler(new tcp_idle_monitor(IDLE_EVENT, 1000, nullptr)); //
	serv.add_channel_handler(new channel_coder_i()); //
	serv.add_channel_handler(new serv_ch_handler_i()); //
	iserv_p = &serv;
	serv.start(10999);

	{
		shan::net::tcp_client cli;
		cli.add_channel_handler(new channel_coder_i()); //
		cli.add_channel_handler(new cli_ch_handler_i()); // 이 핸들러는 cli가 destroy될 때 같이 해제된다.
		icli_p = &cli;
		cli.start();
		cli.connect("127.0.0.1", 10999);
		cli.wait_stop();
	}

	serv.wait_stop();
}
