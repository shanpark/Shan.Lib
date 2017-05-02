//
//  exception.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 15..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_exception_h
#define shan_net_exception_h

#include <exception>

namespace shan {
namespace net {

class net_error : public std::runtime_error {
public:
	explicit net_error(const std::string& what) : std::runtime_error(what) {}
	explicit net_error(const char* what) : std::runtime_error(what) {}
};

class service_error : public net_error {
public:
	explicit service_error(const std::string& what) : net_error(what) {}
	explicit service_error(const char* what) : net_error(what) {}
};

class acceptor_error : public net_error {
public:
	explicit acceptor_error(const std::string& what) : net_error(what) {}
	explicit acceptor_error(const char* what) : net_error(what) {}
};

class channel_error : public net_error {
public:
	explicit channel_error(const std::string& what) : net_error(what) {}
	explicit channel_error(const char* what) : net_error(what) {}
};

class resolver_error : public net_error {
public:
	explicit resolver_error(const std::string& what) : net_error(what) {}
	explicit resolver_error(const char* what) : net_error(what) {}
};

class ssl_error : public net_error {
public:
	explicit ssl_error(const std::string& what) : net_error(what) {}
	explicit ssl_error(const char* what) : net_error(what) {}
};

} // namespace net
} // namespace shan

#endif /* shan_net_exception_h */
