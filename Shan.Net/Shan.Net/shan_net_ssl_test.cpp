//
//  shan_net_ssl_test.cpp
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 5..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <ctime>
#include "net/net_ssl.h"
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

ssl_client* scli_p;
ssl_server* sserv_p;

class acpt_handler_s : public acceptor_handler {
	virtual void user_event(context* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "acpt_handler::" << "user_event() called" << endl;
	}

	virtual void exception_caught(context* ctx, const std::exception& e) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "acpt_handler::" << "exception_caught() - " << e.what() << endl;
	}

	virtual void channel_accepted(acceptor_context* ctx, const std::string& address, uint16_t port) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "acpt_handler::" << "channel_accepted() - " << "connection from '" << address << ":" << port << "'" << endl;
	}
};

class channel_coder_s : public tcp_channel_handler {
	virtual void channel_read(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
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

	virtual void channel_write(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
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

class serv_ch_handler_s : public tcp_channel_handler {
public:
	virtual ~serv_ch_handler_s() {
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

	virtual void channel_connected(tcp_channel_context_base* ctx) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "serv_ch_handler::" << "channel_connected(" << ctx->channel_id() << ") called" << endl;
		}
		auto data = std::make_shared<unix_time>(3000);
		ctx->write(data);
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
//		ctx->close(); // 여기서 close를 하면 데이터는 전송되지 않는다.
	}

	virtual void channel_written(tcp_channel_context_base* ctx, std::size_t bytes_transferred, shan::util::streambuf_ptr sb_ptr) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "serv_ch_handler::" << "channel_written() - " << bytes_transferred << endl;
		}
		ctx->close();
	}

	virtual void channel_disconnected(tcp_channel_context_base* ctx) override {
		static int c = 0;
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "serv_ch_handler::" << "channel_disconnected() called:" << ++c << endl;
		}

		sserv_p->stop();
	}
};

class cli_ch_handler_s : public tcp_channel_handler {
public:
	~cli_ch_handler_s() {
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
		ctx->close();
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

	virtual void channel_written(tcp_channel_context_base* ctx, std::size_t bytes_transferred, shan::util::streambuf_ptr sb_ptr) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << "cli_ch_handler::" << "channel_written() called" << endl;
	}

	virtual void channel_disconnected(tcp_channel_context_base* ctx) override {
		static int c = 0;
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << "cli_ch_handler::" << "channel_disconnected() called:" << ++c << endl;
		}
		scli_p->stop(); // stop()호출 뒤에 발생되는 이벤트는 핸들러 호출이 되지 않는다.
	}
};

std::string get_password(std::size_t max_length, ssl_password_purpose purpose) {
	return "test";
}

bool verify_certificate(bool preverified, X509_STORE_CTX* ctx) {
	// 미리 기본 인증 과정을 태워서 preverified 값을 결정하는 것 같음.
	// 따라서 상용인 경우 true로, 사설인 경우 false로 오는 것 같음...

	// In this example we will simply print the certificate's subject name.
	char subject_name[256];
	X509* cert = X509_STORE_CTX_get_current_cert(ctx);
	X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
	std::cout << "Verifying " << subject_name << "\n";

	return true; // 여기서 true를 반환하면 사설, 상용 상관없이 통과됨.
}

void shan_net_ssl_test() {
	shan::net::ssl_server serv(TLSV12);
	serv.set_options(DEF_OPT | SINGLE_DH_USE | NO_SSLV2);
	serv.set_password_callback(get_password);
	serv.use_certificate_chain_file("/Users/shanpark/Documents/Shan.Lib/Shan.Net/server.pem");
	serv.use_private_key_file("/Users/shanpark/Documents/Shan.Lib/Shan.Net/server.pem", PEM);
	serv.use_tmp_dh_file("/Users/shanpark/Documents/Shan.Lib/Shan.Net/dh2048.pem");

	serv.add_acceptor_handler(new acpt_handler_s()); // 이 핸들러는 serv가 destroy될 때 같이 해제된다. 걱정마라..
	serv.add_channel_handler(new channel_coder_s()); //
	serv.add_channel_handler(new serv_ch_handler_s()); //
	sserv_p = &serv;
	serv.start(10999);
	
	shan::net::ssl_client cli(TLSV12);
//	cli.set_verify_mode(VERIFY_PEER); // 사설 인증서는 통과 안됨.
//	cli.set_verify_callback(verify_certificate); // 여기서 통과시키면 사설 인증서도 통과됨.

	cli.add_channel_handler(new channel_coder_s()); //
	cli.add_channel_handler(new cli_ch_handler_s()); // 이 핸들러는 cli가 destroy될 때 같이 해제된다.
	scli_p = &cli;
	cli.start();
	cli.connect("127.0.0.1", 10999);

	serv.wait_stop();
	cli.wait_stop();
}
