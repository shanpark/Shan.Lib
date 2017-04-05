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


const int DEFAULT_BACKLOG = asio::socket_base::max_connections;
const uint16_t ANY = 0;

using streambuf_pool = util::static_pool<util::streambuf>;

#ifdef SHAN_NET_SSL_ENABLE
enum ssl_method {
	SSLV2         = asio::ssl::context::method::sslv2,         // Generic SSL version 2.
	SSLV2_CLIENT  = asio::ssl::context::method::sslv2_client,  // SSL version 2 client.
	SSLV2_SERVER  = asio::ssl::context::method::sslv2_server,  // SSL version 2 server.
	SSLV3         = asio::ssl::context::method::sslv3,         // Generic SSL version 3.
	SSLV3_CLIENT  = asio::ssl::context::method::sslv3_client,  // SSL version 3 client.
	SSLV3_SERVER  = asio::ssl::context::method::sslv3_server,  // SSL version 3 server.
	TLSV1         = asio::ssl::context::method::tlsv1,         // Generic TLS version 1.
	TLSV1_CLIENT  = asio::ssl::context::method::tlsv1_client,  // TLS version 1 client.
	TLSV1_SERVER  = asio::ssl::context::method::tlsv1_server,  // TLS version 1 server.
	SSLV23        = asio::ssl::context::method::sslv23,        // Generic SSL/TLS.
	SSLV23_CLIENT = asio::ssl::context::method::sslv23_client, // SSL/TLS client.
	SSLV23_SERVER = asio::ssl::context::method::sslv23_server, // SSL/TLS server.
	TLSV11        = asio::ssl::context::method::tlsv11,        // Generic TLS version 1.1.
	TLSV11_CLIENT = asio::ssl::context::method::tlsv11_client, // TLS version 1.1 client.
	TLSV11_SERVER = asio::ssl::context::method::tlsv11_server, // TLS version 1.1 server.
	TLSV12        = asio::ssl::context::method::tlsv12,        // Generic TLS version 1.2.
	TLSV12_CLIENT = asio::ssl::context::method::tlsv12_client, // TLS version 1.2 client.
	TLSV12_SERVER = asio::ssl::context::method::tlsv12_server  // TLS version 1.2 server.
};

using ssl_option = unsigned long;
enum {
	DEF_OPT       = asio::ssl::context::default_workarounds, // Implement various bug workarounds.
	SINGLE_DH_USE = asio::ssl::context::single_dh_use,       // Always create a new key when using tmp_dh parameters.
	NO_SSLV2      = asio::ssl::context::no_sslv2,            // Disable SSL v2.
	NO_SSLV3      = asio::ssl::context::no_sslv3,            // Disable SSL v3.
	NO_TLSV1      = asio::ssl::context::no_tlsv1,            // Disable TLS v1.
	NO_TLSV1_1    = asio::ssl::context::no_tlsv1_1,          // Disable TLS v1.1.
	NO_TLSV1_2    = asio::ssl::context::no_tlsv1_2,          // Disable TLS v1.2.
//	NO_COMPRESSION = asio::ssl::context::no_compression      // Disable compression. Compression is disabled by default.
};

enum ssl_verify_mode {
	VERIFY_NONE                 = asio::ssl::context::verify_none,                 // No verification.
	VERIFY_PEER                 = asio::ssl::context::verify_peer,                 // Verify the peer.
	VERIFY_FAIL_IF_NO_PEER_CERT = asio::ssl::context::verify_fail_if_no_peer_cert, // Fail verification if the peer has no certificate. Ignored unless VERIFY_PEER is set.
	VERIFY_CLIENT_ONCE          = asio::ssl::context::verify_client_once           // Do not request client certificate on renegotiation. Ignored unless VERIFY_PEER is set.
};

enum ssl_file_format {
	ASN1 = asio::ssl::context::file_format::asn1, // ASN.1 file.
	PEM  = asio::ssl::context::file_format::pem,  // PEM file.
};

enum ssl_password_purpose {
	FOR_READING, // The password is needed for reading/decryption.
	FOR_WRITING // The password is needed for writing/encryption.
};

using verify_callback = bool(bool preverified, X509_STORE_CTX* ctx);
using password_callback = std::string(std::size_t max_length, ssl_password_purpose purpose);
#endif

} // namespace net
} // namespace shan

#endif /* shan_net_constants_h */
