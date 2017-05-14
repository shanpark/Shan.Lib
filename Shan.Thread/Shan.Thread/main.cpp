//
//  main.cpp
//  Shan.Thread
//
//  Created by Sung Han Park on 2017. 5. 12..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <shan/thread/worker_pool.h>

#define WORKER_COUNT	7
#define COUNT 			300000

using namespace shan;

std::atomic_int process(0);

void work(void) {
	std::this_thread::sleep_for(std::chrono::microseconds(10));
	process++;
}

int main(int argc, const char * argv[]) {
	std::chrono::time_point<std::chrono::system_clock> start, end;

	start = std::chrono::system_clock::now();
	{
		thread::worker_pool p(WORKER_COUNT);
		process = 0;
		for (int inx = 0 ; inx < COUNT ; inx++) {
			p.send(work);
		}
		p.wait_complete();
		end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::cout << WORKER_COUNT << " Shan Thread time: " << elapsed_seconds.count() << std::endl;
		std::cout << "process:" << process << std::endl;
		p.stop(true);
	}

//	start = std::chrono::system_clock::now();
//	{
//		process = 0;
//		for (int inx = 0 ; inx < COUNT ; inx++) {
//			work();
//		}
//	}
//	end = std::chrono::system_clock::now();
//	elapsed_seconds = end-start;
//	std::cout << "No thread time: " << elapsed_seconds.count() << std::endl;
//	std::cout << "process:" << process << std::endl;

	return 0;
}

