//
//  constants.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 16..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_constants_h
#define shan_net_constants_h

namespace shan {
namespace net {

class protocol {
public:
	class tcp {};
	class udp {};
};

enum class ip {
	v4,
	v6
};

const uint16_t ANY = 0;

using streambuf_pool = util::static_pool<util::streambuf>;

} // namespace net
} // namespace shan

#endif /* shan_net_constants_h */
