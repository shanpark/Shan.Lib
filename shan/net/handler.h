//
//  handler.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_handler_h
#define shan_net_handler_h

namespace shan {
namespace net {

class context_base;

class handler : public object {
public:
	virtual void user_event(shan::net::context_base* ctx) {} // <-- inbound
	virtual void exception_caught(shan::net::context_base* ctx, const std::exception& e) {} // <-- inbound
};

} // namespace net
} // namespace shan

#include "acceptor_handler.h"
#include "channel_handler.h"

#endif /* shan_net_handler_h */
