//
//  main.cpp
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include "net/net.h"

using namespace std;
using namespace shan::net;

class unix_time : public shan::object {
public:
	unix_time(uint64_t time) { _time = time; }
	std::time_t get_time() { return _time; }

public:
	std::time_t _time;
};

std::mutex _mutex;

client* cli_p;
server* serv_p;

class acpt_handler : public acceptor_handler {
	virtual void user_event(context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "acpt_handler::" << "user_event() called" << endl;
	}

	virtual void exception_caught(const std::exception& e, context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "acpt_handler::" << "exception_caught() - " << e.what() << endl;
	}

	virtual void channel_accepted(acceptor_context* ctx, const std::string& address, uint16_t port) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "acpt_handler::" << "channel_accepted() - " << "connection from '" << address << ":" << port << "'" << endl;
	}
};

class channel_coder : public channel_handler {
	virtual void channel_read(channel_context* ctx, shan::object_ptr& data) {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto sb_ptr = static_pointer_cast<shan::util::streambuf>(data);
		if (sb_ptr->in_size() < sizeof(std::time_t)) { // not enough data
			ctx->done(true);
			return;
		}

		std::time_t time;
		sb_ptr->read_int64(reinterpret_cast<int64_t*>(&time));

		data = std::make_shared<unix_time>(time);
	}

	virtual void channel_write(channel_context* ctx, shan::object_ptr& data) {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto time_ptr = static_pointer_cast<unix_time>(data);

		auto sb_ptr = std::make_shared<shan::util::streambuf>(sizeof(std::time_t));
		sb_ptr->write_int64(time_ptr->get_time());
		data = sb_ptr;
	}
};

class serv_ch_handler : public channel_handler {
public:
	virtual ~serv_ch_handler() {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << ">>>> serv_ch_handler destroyed!!!!" << endl;
	};

	virtual void user_event(context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "user_event() called" << endl;
	}

	virtual void exception_caught(const std::exception& e, context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "exception_caught() - " << e.what() << endl;
	}

	virtual void channel_connected(channel_context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_connected(" << ctx->channel_id() << ") called" << endl;
		auto data = std::make_shared<unix_time>(3000);
		ctx->write(data);
		ctx->done(true);
	}

	virtual void channel_read(channel_context* ctx, shan::object_ptr& data) {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto sb_ptr = static_pointer_cast<shan::util::streambuf>(data);
		cout << "serv_ch_handler::" << "channel_read() - " << sb_ptr->in_size() << endl;
		sb_ptr->consume(sb_ptr->in_size());
	}

	virtual void channel_read_complete(channel_context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_read_complete() called" << endl;
	}

	virtual void channel_write(channel_context* ctx, shan::object_ptr& data) {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto sb_ptr = static_pointer_cast<shan::util::streambuf>(data);
		cout << "serv_ch_handler::" << "channel_write() - " << sb_ptr->in_size() << endl;
//		ctx->close(); // 여기서 close를 하면 데이터는 전송되지 않는다.
	}

	virtual void channel_written(channel_context* ctx, std::size_t bytes_transferred, shan::util::streambuf_ptr sb_ptr) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_written() - " << bytes_transferred << endl;
		ctx->close();
	}

	virtual void channel_disconnected(channel_context* ctx) {
		static int c = 0;
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "serv_ch_handler::" << "channel_disconnected() called:" << ++c << endl;

		serv_p->stop();
	}
};

class cli_ch_handler : public channel_handler {
public:
	~cli_ch_handler() {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << ">>>> cli_ch_handler destroyed" << endl;
	};

	virtual void user_event(context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "user_event() called" << endl;
	}

	virtual void exception_caught(const std::exception& e, context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "exception_caught() - " << e.what() << endl;
	}

	virtual void channel_connected(channel_context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_connected(" << ctx->channel_id() << ") called" << endl;
	}

	virtual void channel_read(channel_context* ctx, shan::object_ptr& data) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_read() called" << endl;
		auto time = static_pointer_cast<unix_time>(data);
		cout << "time:" << time->get_time() << endl;
		ctx->close();
	}

	virtual void channel_read_complete(channel_context* ctx) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_read_complete() called" << endl;
	}

	virtual void channel_write(channel_context* ctx, shan::object_ptr& data) {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto sb_ptr = static_pointer_cast<shan::util::streambuf>(data);
		cout << "cli_ch_handler::" << "channel_write() - " << sb_ptr->in_size() << endl;
	}

	virtual void channel_written(channel_context* ctx, std::size_t bytes_transferred, shan::util::streambuf_ptr sb_ptr) {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_written() called" << endl;
	}

	virtual void channel_disconnected(channel_context* ctx) {
		static int c = 0;
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_disconnected() called:" << ++c << endl;
		cli_p->stop(); // stop()호출 뒤에 발생되는 이벤트는 핸들러 호출이 되지 않는다.
	}
};

void shan_net_test() {
	shan::net::server serv;
	serv.add_acceptor_handler(new acpt_handler()); // 이 핸들러는 serv가 destroy될 때 같이 해제된다. 걱정마라..
	serv.add_channel_handler(new channel_coder()); //
	serv.add_channel_handler(new serv_ch_handler()); //
	serv_p = &serv;
	serv.start(10999);

//	{
		shan::net::client cli;
		cli.add_channel_handler(new cli_ch_handler()); // 이 핸들러는 cli가 destroy될 때 같이 해제된다.
		cli.add_channel_handler(new channel_coder()); //
		cli_p = &cli;
		cli.start();
		cli.connect("127.0.0.1", 10999);

//		getchar();
//		cli.wait_stop();
//		cli.stop();
//		cout << "client stopped" << endl;
//	}

//	getchar();
	serv.wait_stop();
	cli.wait_stop();
//	serv.stop();

//	cout << "server stopped" << endl;
}

void streambuf_test() {
	shan::util::streambuf sb(128);

	std::ostream os(&sb);
	os << "0-2345678901-3456789012-4567890123-5678901234-6789";
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	os << "0-2345678901-3456789012-4567890123-5678901234-6789";
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	os << "0-2345678901-3456789012-4567890123-5678901234-6789";
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	os << "0-2345678901-3456789012-4567890123-5678901234-6789";
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	os << "0-2345678901-3456789012-4567890123-5678901234-6789";
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	os << "0-2345678901-3456789012-4567890123-5678901234-6789";
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;
	
	sb.consume(150);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	sb.consume(150);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	os << "0-2345678901-3456789012-4567890123-5678901234-6789";
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	sb.consume(50);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	sb.prepare(50);
	sb.commit(50);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	sb.prepare(50);
	sb.commit(50);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	sb.prepare(50);
	sb.commit(50);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	sb.prepare(50);
	sb.commit(50);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	sb.prepare(50);
	sb.commit(50);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	sb.prepare(50);
	sb.commit(50);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;

	sb.prepare(50);
	sb.commit(50);
	std::cout << sb.in_size() << "/" << sb.alloc_size() << std::endl;
}

int main(int argc, const char * argv[]) {
//	streambuf_test();
	shan_net_test();

	return 0;
}
