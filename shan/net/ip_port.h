//
//  address.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 29..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_address_h
#define shan_net_address_h

namespace shan {
namespace net {

class ip_port : public object {
	friend class udp_channel;
	friend class udp_service;
public:
	ip_port() = default; // unspecified address
	ip_port(const std::string& ip_str, uint16_t port)
	: _address(), _port(port) {
		if (ip_str.length() > 0)
			_address = asio::ip::address::from_string(ip_str);
	}

public:
	bool is_valid() const { return !_address.is_unspecified(); }
	
	std::string ip() const { return _address.to_string(); };
	uint16_t port() const { return _port; }

private:
	ip_port(asio::ip::address address, uint16_t port)
	: _address(address), _port(port) {}
	void set(asio::ip::address address, uint16_t port) {
		_address = address;
		_port = port;
	}
	asio::ip::udp::endpoint udp_endpoint() const {
		return asio::ip::udp::endpoint(_address, _port);
	}

private:
	asio::ip::address _address;
	uint16_t _port;
};

} // namespace net
} // namespace shan

#endif /* shan_net_address_h */
