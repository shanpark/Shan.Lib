//
//  main.cpp
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include "util/util.h"
#include "util/pool.h"
#include "util/streambuf.h"
#include "shan_net_test.h"

#include "net/net_ssl.h"

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
