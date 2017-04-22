//
//  ssl_service_base.h
//  Shan.Net
//
//  Created by Sung Han Park on 2017. 4. 4..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_net_ssl_service_base_h
#define shan_net_ssl_service_base_h

namespace shan {
namespace net {

class ssl_service_base : public object {
public:
	ssl_service_base(ssl_method method) : _ssl_context(static_cast<asio::ssl::context::method>(method)) {}

	virtual bool is_running() = 0;

	/**
	 Clear options on the context.
	 This function may be used to configure the SSL options used by the context.
	 Calls SSL_CTX_clear_options.
	 @param options 
	 A bitmask of options. The available option values are defined in enum 
	 ssl_option. The specified options, if currently enabled on the context, 
	 are cleared.
	 */
	void clear_options(ssl_option options) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.clear_options(options);
		} catch (const std::exception& e) {
			throw ssl_error(std::string("clear_options() failed. (") + e.what() + ")");
		}
	}

	/**
	 Set options on the context.
	 This function may be used to configure the SSL options used by the context.
	 Calls SSL_CTX_set_options.
	 @param options
	 A bitmask of options. The available option values are defined in enum
	 ssl_option. The options are bitwise-ored with any existing value for the 
	 options.
	 */
	void set_options(ssl_option options) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.set_options(options);
		} catch (const std::exception& e) {
			throw ssl_error(std::string("set_options() failed. (") + e.what() + ")");
		}
	}

	/**
	 Set the peer verification mode.
	 This function may be used to configure the peer verification mode used by 
	 the context.
	 Calls SSL_CTX_set_verify.
	 @param v
	 A bitmask of peer verification modes. See enum ssl_verify_mode for
	 available values.
	 */
	void set_verify_mode(ssl_verify_mode v) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.set_verify_mode(v);
		} catch (const std::exception& e) {
			throw ssl_error(std::string("set_verify_mode() failed. (") + e.what() + ")");
		}
	}

	/**
	 Set the peer verification depth.
	 This function may be used to configure the maximum verification depth 
	 allowed by the context.
	 Calls SSL_CTX_set_verify_depth.
	 @param depth
	 Maximum depth for the certificate chain verification that shall be allowed.
	 */
	void set_verify_depth(int depth) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.set_verify_depth(depth);
		} catch (const std::exception& e) {
			throw ssl_error(std::string("set_verify_depth() failed. (") + e.what() + ")");
		}
	}

	/**
	 Load a certification authority file for performing verification.
	 This function is used to load one or more trusted certification authorities
	 from a file.
	 Calls SSL_CTX_load_verify_locations.
	 @param filename
	 The name of a file containing certification authority certificates in PEM 
	 format.
	 */
	void load_verify_file(const std::string& filename) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.load_verify_file(filename);
		} catch (const std::exception& e) {
			throw ssl_error(std::string("load_verify_file() failed. (") + e.what() + ")");
		}
	}

	/**
	 Add certification authority for performing verification.
	 This function is used to add one trusted certification authority from a 
	 memory buffer.
	 Calls SSL_CTX_get_cert_store and X509_STORE_add_cert.
	 @param ca
	 The buffer containing the certification authority certificate. The 
	 certificate must use the PEM format.
	 */
	void add_certificate_authority(const char* ca, int ca_len) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.add_certificate_authority(asio::buffer(ca, ca_len));
		} catch (const std::exception& e) {
			throw ssl_error(std::string("add_certificate_authority() failed. (") + e.what() + ")");
		}
	}

	/**
	 Configures the context to use the default directories for finding 
	 certification authority certificates.
	 This function specifies that the context should use the default, 
	 system-dependent directories for locating certification authority 
	 certificates.
	 Calls SSL_CTX_set_default_verify_paths.
	 */
	void set_default_verify_paths() {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.set_default_verify_paths();
		} catch (const std::exception& e) {
			throw ssl_error(std::string("set_default_verify_paths() failed. (") + e.what() + ")");
		}
	}

	/**
	 Add a directory containing certificate authority files to be used for 
	 performing verification.
	 This function is used to specify the name of a directory containing 
	 certification authority certificates. Each file in the directory must 
	 contain a single certificate. The files must be named using the subject 
	 name's hash and an extension of ".0".
	 Calls SSL_CTX_load_verify_locations.
	 @param path
	 The name of a directory containing the certificates
	 */
	void add_verify_path(const std::string& path) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.add_verify_path(path);
		} catch (const std::exception& e) {
			throw ssl_error(std::string("add_verify_path() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use a certificate from a memory buffer.
	 This function is used to load a certificate into the context from a buffer.
	 Calls SSL_CTX_use_certificate or SSL_CTX_use_certificate_ASN1.
	 @param cert
	 The buffer containing the certificate.
	 @param cert_len
	 The length of the buffer containing the certificate.
	 @param format
	 The certificate format (ASN1 or PEM). See enum ssl_file_format for
	 available values.
	 */
	void use_certificate(const uint8_t* cert, std::size_t cert_len, ssl_file_format format) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_certificate(asio::buffer(cert, cert_len), static_cast<asio::ssl::context::file_format>(format));
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_certificate() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use a certificate from a file.
	 This function is used to load a certificate into the context from a file.
	 Calls SSL_CTX_use_certificate_file.
	 @param filename
	 The name of the file containing the certificate.
	 @param format
	 The file format (ASN1 or PEM). See enum ssl_file_format for available 
	 values.
	 */
	void use_certificate_file(const std::string& filename, ssl_file_format format) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_certificate_file(filename, static_cast<asio::ssl::context::file_format>(format));
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_certificate_file() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use a certificate chain from a memory buffer.
	 This function is used to load a certificate chain into the context from a 
	 buffer.
	 Calls SSL_CTX_use_certificate and SSL_CTX_add_extra_chain_cert.
	 @param chain
	 The buffer containing the certificate chain. The certificate chain must use
	 the PEM format.
	 @param chain_len
	 The length of the buffer containing the certificate chain.
	 */
	void use_certificate_chain(const uint8_t* chain, std::size_t chain_len) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_certificate_chain(asio::buffer(chain, chain_len));
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_certificate_chain() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use a certificate chain from a file.
	 This function is used to load a certificate chain into the context from a 
	 file.
	 Calls SSL_CTX_use_certificate_chain_file.
	 @param filename
	 The name of the file containing the certificate. The file must use the PEM 
	 format.
	 */
	void use_certificate_chain_file(const std::string& filename) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_certificate_chain_file(filename);
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_certificate_chain_file() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use a private key from a memory buffer.
	 This function is used to load a private key into the context from a buffer.
	 Calls SSL_CTX_use_PrivateKey or SSL_CTX_use_PrivateKey_ASN1.
	 @param private_key
	 The buffer containing the private key.
	 @param private_key_len
	 The length of the buffer containing the private key.
	 @param format
	 The private key format (ASN1 or PEM). See enum ssl_file_format for 
	 available values.
	 */
	void use_private_key(const uint8_t* private_key, std::size_t private_key_len, ssl_file_format format) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_private_key(asio::buffer(private_key, private_key_len), static_cast<asio::ssl::context::file_format>(format));
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_private_key() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use a private key from a file.
	 This function is used to load a private key into the context from a file.
	 Calls SSL_CTX_use_PrivateKey_file.
	 @param filename
	 The name of the file containing the private key.
	 @param format
	 The file format (ASN1 or PEM). See enum ssl_file_format for available 
	 values.
	 */
	void use_private_key_file(const std::string& filename, ssl_file_format format) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_private_key_file(filename, static_cast<asio::ssl::context::file_format>(format));
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_private_key_file() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use an RSA private key from a memory buffer.
	 This function is used to load an RSA private key into the context from a 
	 buffer.
	 Calls SSL_CTX_use_RSAPrivateKey or SSL_CTX_use_RSAPrivateKey_ASN1.
	 @param private_key
	 The buffer containing the RSA private key.
	 @param private_key_len
	 The length of the buffer containing the RSA private key.
	 format
	 The private key format (ASN.1 or PEM). See enum ssl_file_format for 
	 available values.
	 */
	void use_rsa_private_key(const uint8_t* private_key, std::size_t private_key_len, ssl_file_format format) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_rsa_private_key(asio::buffer(private_key, private_key_len), static_cast<asio::ssl::context::file_format>(format));
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_rsa_private_key() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use an RSA private key from a file.
	 This function is used to load an RSA private key into the context from a 
	 file.
	 Calls SSL_CTX_use_RSAPrivateKey_file.
	 @param filename
	 The name of the file containing the RSA private key.
	 @param format
	 The file format (ASN1 or PEM). See enum ssl_file_format for available
	 values.
	 */
	void use_rsa_private_key_file(const std::string& filename, ssl_file_format format) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_rsa_private_key_file(filename, static_cast<asio::ssl::context::file_format>(format));
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_rsa_private_key_file() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use the specified memory buffer to obtain the temporary Diffie-Hellman 
	 parameters.
	 This function is used to load Diffie-Hellman parameters into the context 
	 from a buffer.
	 Calls SSL_CTX_set_tmp_dh.
	 @param dh
	 The memory buffer containing the Diffie-Hellman parameters. The buffer must
	 use the PEM format.
	 @param dh_len
	 The length of the memory buffer containing the Diffie-Hellman parameters.
	 */
	void use_tmp_dh(const uint8_t* dh, std::size_t dh_len) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_tmp_dh(asio::buffer(dh, dh_len));
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_tmp_dh() failed. (") + e.what() + ")");
		}
	}

	/**
	 Use the specified file to obtain the temporary Diffie-Hellman parameters.
	 This function is used to load Diffie-Hellman parameters into the context 
	 from a file.
	 Calls SSL_CTX_set_tmp_dh.
	 @param filename
	 The name of the file containing the Diffie-Hellman parameters. The file
	 must use the PEM format.
	 */
	void use_tmp_dh_file(const std::string& filename) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.use_tmp_dh_file(filename);
		} catch (const std::exception& e) {
			throw ssl_error(std::string("use_tmp_dh_file() failed. (") + e.what() + ")");
		}
	}

	/**
	 Set the callback used to verify peer certificates.
	 This function is used to specify a callback function that will be called by
	 the implementation when it needs to verify a peer certificate.
	 Calls SSL_CTX_set_verify.
	 @param verify_cb
	 The function object to be used for verifying a certificate. 
	 The function signature of the handler must be:
	   bool verify_callback(
         bool preverified,   // True if the certificate passed pre-verification.
         verify_context& ctx // The peer certificate and other context.
	   );
	 The return value of the callback is true if the certificate has passed 
	 verification, false otherwise.
	 */
	void set_verify_callback(std::function<verify_callback> verify_cb) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.set_verify_callback([verify_cb](bool preverified, asio::ssl::verify_context& ctx){
				return verify_cb(preverified, ctx.native_handle());
			});
		} catch (const std::exception& e) {
			throw ssl_error(std::string("set_verify_callback() failed. (") + e.what() + ")");
		}
	}

	/**
	 Set the password callback.
	 This function is used to specify a callback function to obtain password 
	 information about an encrypted key in PEM format.
	 Calls SSL_CTX_set_default_passwd_cb.
	 @param pw_cb
	 The function object to be used for obtaining the password. The function 
	 signature of the handler must be:
	   std::string password_callback(
	     std::size_t max_length,  // The maximum size for a password.
	     password_purpose purpose // Whether password is for reading or writing.
	   );
	 The return value of the callback is a string containing the password.
	 */
	void set_password_callback(std::function<password_callback> pw_cb) {
		try {
			if (is_running())
				throw ssl_error("ssl service is already running.");
			_ssl_context.set_password_callback([pw_cb](std::size_t max_length, asio::ssl::context::password_purpose purpose){
				return pw_cb(max_length, static_cast<ssl_password_purpose>(purpose));
			});
		} catch (const std::exception& e) {
			throw ssl_error(std::string("set_password_callback() failed. (") + e.what() + ")");
		}
	}

protected:
	asio::ssl::context _ssl_context;
};

} // namespace net
} // namespace shan

#endif /* shan_net_ssl_service_base_h */
