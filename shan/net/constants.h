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

enum class protocol {
	tcp,
	udp
};

enum class mode {
	server,
	client
};

enum class ip {
	v4,
	v6
};
	
} // namespace net
} // namespace shan

#endif /* shan_net_constants_h */
