//
//  main.cpp
//  SSLClient
//
//  Created by Sung Han Park on 2017. 4. 11..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <shan/net_ssl.h>
#include <shan/util/pool.h>

using namespace std;
using namespace shan::net;

#ifdef RASPI_TEST
#define CLIENT_COUNT 10000
#else
#define CLIENT_COUNT 50000
#endif

class unix_time : public shan::object {
public:
	unix_time(std::time_t time) { _time = time; }
	std::time_t get_time() { return _time; }

public:
	std::time_t _time;
};

std::mutex _mutex;
int conn = 0;
int dis_conn = 0;
int rd = 0;
int exc = 0;

ssl_client* scli_p;

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
			time = sb_ptr->read_int32();
		else
			time = sb_ptr->read_int64();

		data = std::make_shared<unix_time>(time);
	}

	virtual void channel_write(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		auto time_ptr = static_cast<unix_time*>(data.get());

		auto sb_ptr = std::make_shared<shan::util::streambuf>(sizeof(std::time_t));
		if (sizeof(std::time_t) == 4)
			sb_ptr->write_int32(static_cast<int32_t>(time_ptr->get_time()));
		else
			sb_ptr->write_int64(static_cast<int32_t>(time_ptr->get_time()));

		data = sb_ptr; // return new converted object.
	}
};

class cli_ch_handler_s : public tcp_channel_handler {
public:
	~cli_ch_handler_s() {
//		std::lock_guard<std::mutex> _lock(_mutex);
//		cout << "!!!! cli_ch_handler destroyed" << endl;
	};

	virtual void user_event(tcp_channel_context_base* ctx, int64_t id, shan::object_ptr data_ptr) override {
//		std::lock_guard<std::mutex> _lock(_mutex);
//		cout << static_cast<ssl_channel_context*>(ctx)->channel_id() << ":cli_ch_handler::" << "user_event() called" << endl;
	}

	virtual void exception_caught(tcp_channel_context_base* ctx, const std::exception& e) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			exc++;
			cout << static_cast<ssl_channel_context*>(ctx)->channel_id() << ":" << std::this_thread::get_id() << ":cli_ch_handler::" << "exception_caught() - " << e.what() << "@@@@@@@@@@@@@@@@@@@@@" << endl;
		}

		if (dis_conn == (CLIENT_COUNT - exc)) {
			cout << "SSLClient request stop [exc] @!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@" << endl;
			scli_p->request_stop(); // stop()호출 뒤에 발생되는 이벤트는 핸들러 호출이 되지 않는다.
		}
	}

	virtual void channel_connected(tcp_channel_context_base* ctx) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			conn++;
			cout << static_cast<ssl_channel_context*>(ctx)->channel_id() << ":" << std::this_thread::get_id() << ":cli_ch_handler::" << "channel_connected(" << ctx->channel_id() << ") called" << endl;
		}
	}

	virtual void channel_read(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			cout << static_cast<ssl_channel_context*>(ctx)->channel_id() << ":" << std::this_thread::get_id() << ":cli_ch_handler::" << "channel_read() called" << endl;
//			auto time = static_cast<unix_time*>(data.get());
//			cout << "time:" << time->get_time() << endl;
			rd++;
		}

		ctx->close();
	}

	virtual void channel_rdbuf_empty(tcp_channel_context_base* ctx) override {
		std::lock_guard<std::mutex> _lock(_mutex);
		cout << static_cast<ssl_channel_context*>(ctx)->channel_id() << ":" << std::this_thread::get_id() << ":cli_ch_handler::" << "channel_rdbuf_empty() called" << endl;
	}

	virtual void channel_write(tcp_channel_context_base* ctx, shan::object_ptr& data) override {
//		std::lock_guard<std::mutex> _lock(_mutex);
//		auto sb_ptr = static_cast<shan::util::streambuf*>(data.get());
//		cout << static_cast<ssl_channel_context*>(ctx)->channel_id() << ":cli_ch_handler::" << "channel_write() - " << sb_ptr->in_size() << endl;
	}

	virtual void channel_written(tcp_channel_context_base* ctx, std::size_t bytes_transferred) override {
//		std::lock_guard<std::mutex> _lock(_mutex);
//		cout << static_cast<ssl_channel_context*>(ctx)->channel_id() << ":cli_ch_handler::" << "channel_written() called" << endl;
	}

	virtual void channel_disconnected(tcp_channel_context_base* ctx) override {
		{
			std::lock_guard<std::mutex> _lock(_mutex);
			dis_conn++;
			cout << static_cast<ssl_channel_context*>(ctx)->channel_id() << ":" << std::this_thread::get_id() << ":cli_ch_handler::" << "channel_disconnected() called:" << dis_conn << endl;

			if (dis_conn == (CLIENT_COUNT - exc)) {
				cout << "SSLClient request stop [disconn]@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@" << endl;
				scli_p->request_stop(); // stop()호출 뒤에 발생되는 이벤트는 핸들러 호출이 되지 않는다.
			}
		}
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

int main(int argc, const char * argv[]) {
	shan::net::ssl_client cli(TLSV12);
//	shan::net::tcp_client cli;

	cli.add_channel_handler(new channel_coder_s()); //
	cli.add_channel_handler(new cli_ch_handler_s()); // 이 핸들러는 cli가 destroy될 때 같이 해제된다.
	scli_p = &cli;
	cli.start();

	cout << "SSLClient start" << endl;

#ifdef RASPI_TEST
	std::chrono::milliseconds dura(9);
#endif
#ifdef WIN64_TEST
	std::chrono::milliseconds dura(20);
#endif
#ifdef MACOS_TEST
	std::chrono::milliseconds dura(1);
#endif
	for (int inx = 0 ; inx < CLIENT_COUNT ; inx++) {
		cli.connect("127.0.0.1", 10888);
		std::this_thread::sleep_for(dura);
	}

//	cout << "SSLClient connect end @!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@!@" << endl;
	cli.wait_stop();

	cout << "conn:" << conn << ", dis_conn:" << dis_conn << ", exc:" << exc << ", read:" << rd << endl;
}
