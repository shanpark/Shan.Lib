//
//  acceptor_handler.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 16..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_acceptor_handler_h
#define shan_net_acceptor_handler_h

namespace shan {
namespace net {

class acceptor_context;

class acceptor_handler : public handler {
public:
	virtual void channel_accepted(acceptor_context* ctx, const std::string& address, uint16_t port) {} // <-- inbound
};

} // namespace net
} // namespace shan

#endif /* shan_net_server_handler_h */
