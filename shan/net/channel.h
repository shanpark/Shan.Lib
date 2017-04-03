//
//  channel.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 3. 14..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_channel_h
#define shan_net_channel_h

namespace shan {
namespace net {

using connect_complete_handler = void (const asio::error_code& error);
using read_complete_handler = void (const asio::error_code& error, util::streambuf_ptr sb_ptr);
using write_complete_handler = void (const asio::error_code& error, std::size_t bytes_transferred, util::streambuf_ptr sb_ptr);

template<typename Protocol>
class channel_context;

template<typename Protocol>
class channel : public object {
	friend class channel_context<Protocol>;
public:
	using ptr = std::unique_ptr<channel>;

protected:
	virtual std::size_t id() const = 0;

	virtual void open(ip v) = 0;
	virtual void close() noexcept = 0;

	virtual void connect(const std::string& address, uint16_t port, std::function<connect_complete_handler> connect_handler) = 0;

	virtual void read(std::function<read_complete_handler> read_handler) noexcept = 0;
	virtual void write_streambuf(util::streambuf_ptr write_sb_ptr, std::function<write_complete_handler> write_handler) = 0;

	virtual asio::io_service& io_service() = 0;
};

} // namespace net
} // namespace shan

#include "tcp_channel.h"
#include "udp_channel.h"
#endif /* shan_net_channel_h */
