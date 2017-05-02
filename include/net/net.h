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

#include "../object.h"
#include "util/util.h"
#include "util/pool.h"
#include "util/streambuf.h"
#include "constants.h"
#include "exception.h"
#include "ip_port.h"
#include "acceptor_handler.h"
#include "channel_handler.h"
#include "handler_pipeline.h"

#include "acceptor.h"
#include "channel_base.h"
#include "context_base.h"

#include "tcp_idle_monitor.h"

#endif /* shan_net_h */
