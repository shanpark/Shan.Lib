//
//  main.cpp
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

// ssl::stream의 async_read()를 호출하면 임의의 스레드에서 SSL_read()가 호출된 후 read가 완료되면
// read_complete가 호출된다. 여기서 async_read()와 SSL_read() 사이에 async_shutdown이 호출되면
// SSL_read()가 호출될 때 bad access가 발생한다. 즉, async_shutdown()을 호출하기 전에는 read뿐 아니라
// 모든 async 작업(shakehand, read, write, connect)을 종료 시키고 호출해야 문제가 없다.

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
