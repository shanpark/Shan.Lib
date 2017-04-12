//
//  main.cpp
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//
// 1. shutdown 후 close 됐는데도 channel_disconnected가 발생하지 않았다. shutdown_complete는
//    정상적으로 호출되었으니까 close는 됐겠지... close후에 disconnect가 발생하지 않으면 적당히 기다렸다가
//    강제로 channel_disconnected를 발생시켜야 하나..
// 2. 어느 한 worker 스레드에서 handler를 수행하고 있었고 그 handler내에서 stop이 호출되었는데
//    다른 스레드에서 작업중인 소켓이 close되면서 그 스레드에서 bad access가 발생한는 것으로 보임.
//    모든 context의 close를 handler_strand를 통해서 호출되도록 하고 그 다음 모든 context를 삭제토록 한 후에
//    삭제가 되면 io_service를 stop() 시키고 모든 스레드가 종료될 때 까지 기다렸다가 worker를 clear하고
//    완전 종료가 되면 cv.notify_all()를 호출하고 끝낸다.
//    stop은 req_stop()이 되겠구나..
// 3. tcp client에서도 connecged는 됐는데 disconnected가 발생하지 않은 채로 남아있는 애들이 있던데 그것도 같은 문제인지...
//    남아있는 context의 상태를 보면 모두 CONNECTED(4)인 것 같다. 아닌 것을 찾지 못했음.
//    그렇다면 read도 못했을 가능성이 있고 close도 받지 못했고 그래서 그냥 가만히 있는 것이라는 건데..
//    실제 서비스라면 connect 후 내가 뭘 요청하거나 받아야할 데이터를 기다리거나 해서 타임 아웃을 기다리는
//    상황이 발생해서 종료되어야 맞겠지..

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

//	std::cout << "========== TCP test ==========" << std::endl;
//	shan_net_tcp_test();

	std::cout << std::endl << "========== SSL test ==========" << std::endl;
	shan_net_ssl_test();

//	std::cout << std::endl << "========== UDP test ==========" << std::endl;
//	shan_net_udp_test();

	return 0;
}
