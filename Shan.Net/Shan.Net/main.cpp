//
//  main.cpp
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//
// openssl 1.1.0e 버전으로 몇 만 번을 접속, 전송, 끊기 테스트를 하면 아래 현상이 나온다.
// ssl_stream의 shutdown을 이용해서 close를 하도록 하면 server, client 양쪽에서 shutdown이
// 호출되었을 때 서버쪽에서 비정상 종료가 일어난다. (bad access)
// ssl_stream의 async_shutdown을 이용해서 close를 하도록 하면 server, client 양쪽에서
// shutdown이 호출되었을 때 malloc_error_debug가 발생한다. 현상은 다르지만 같은 이유로 발생하는
// 것 같다. asio가 1.1.0 버전 openssl을 아직 제대로 처리하지 못하는 것으로 보이는데 인터넷에도
// 정보를 찾을 수가 없다. 대신 SSL_shutdown을 native_handler로 직접 호출하여 unidirection
// shutdown을 하도록하면 아무 문제가 없이 동작한다.
// SSL_shutdown을 직접 호출하더라도 1.11.0-master 소스는 끊어질 때 거의 stream truncated에러가 발생한다.
// 1.10.8 소스는 거의 발생하지 않는다. 가장 안정적인 조합은 현재 아래 조합이다.
// 안정 버전: asio 1.10.8 + openssl 1.1.0e + unidirection shutdown

// openssl 1.0.2k 버전으로 테스트하면 상당히 안정적이지만 속도면에서 약간 느리다.
// handshake가 느려서인지 송수신이 느려서인지 클라이언트 입장에 접속 성공한 요청들은 깔끔하게 처리되지만
// 많은 수의 연결요청이 timeout 또는 aborted 된다. 하지만 양쪽 모두 예외 처리만 깔끔하다면 전체적으로
// 훨씬 안정적이고 stream trucated, Bronke pipe 같은 예외도 거의 발생하지 않고 crash도 당연히 없다.
// 안정 버전: asio 1.10.6 + openssl 1.0.2k + bidirection shutdown
// 안정 버전: asio 1.10.8 + openssl 1.0.2k + bidirection shutdown
// 안정 버전: asio 1.11.0-master + openssl 1.0.2k + bidirection shutdown

// 참고로 Raspberry pi에서는 openssl 1.1.0e 버전도 아무 문제없이 잘 동작한다.

// written에서 close를 호출하면 fire_channel_close가 호출되고 strand에서 close_gracefully가
// 호출된다. 이 때 strand로 post된 lambda가 해제되면서 캡쳐되었던 ch_ctx_ptr도 해제된다.
// 문제는 그 후에 async read 요청이 끝나면서 SSL_read()가 호출이 되어버린다는 것이다.
// 아마도 ch_ctx_ptr이 그대로 남아있었더라면?? 정상적으로 에러가 되면서 read_complete가 error값과
// 함께 호출되지 않을까? read_complete에는 ch_ctx_ptr을 캡쳐해서 보내지 않나? 여튼 원인이 맞다면
// read()가 호출될 때 캡쳐되서 보존되도록 하는 것도 방법일 것같다.
//
// 혹시 close된 소캣에 SSL_read() 호출되서 죽는 거라면 cancel이 호출되서 read_complete가 호출될 때 까지
// close()를 미루는 방법밖에는 없다.

#include <stdio.h>
#include <iostream>
#include "util/util.h"
#include "util/pool.h"
#include "util/streambuf.h"
#include "shan_net_test.h"

void streambuf_test() {
	auto sbp = shan::util::static_pool<shan::util::streambuf>::get_object(128);
	shan::util::streambuf& sb = *sbp;

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

	shan::util::static_pool<shan::util::streambuf>::return_object(sbp);
}

int main(int argc, const char * argv[]) {
//	streambuf_test();

	std::cout << "========== TCP test ==========" << std::endl;
	shan_net_tcp_test();

	std::cout << std::endl << "========== SSL test ==========" << std::endl;
	shan_net_ssl_test();

	std::cout << std::endl << "========== UDP test ==========" << std::endl;
	shan_net_udp_test();

	return 0;
}
