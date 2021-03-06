//
//  net.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_h
#define shan_net_h

#define ASIO_STANDALONE
#include <asio.hpp>

#ifdef SHAN_NET_SSL_ENABLE
#include <asio/ssl.hpp>
#endif

#include <cstdint>
#include <cassert>
#include <string>
#include <memory>
#include <streambuf>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <vector>
#include <deque>
#include <utility>
#include <chrono>

#include <shan/object.h>
#include <shan/util/util.h>
#include <shan/util/pool.h>
#include <shan/util/streambuf.h>
#include <shan/net/constants.h>
#include <shan/net/exception.h>
#include <shan/net/ip_port.h>
#include <shan/net/acceptor_handler.h>
#include <shan/net/channel_handler.h>
#include <shan/net/handler_pipeline.h>

#include <shan/net/acceptor.h>
#include <shan/net/channel_base.h>
#include <shan/net/context_base.h>

#include <shan/net/tcp_idle_monitor.h>

#endif /* shan_net_h */
