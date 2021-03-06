//
//  string.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_string_h
#define shan_json_string_h

namespace shan {
namespace json {

class string : public value, public string_base {
public:
	string() {};
	string(const char* str) : std::string(str) {}
	string(const std::string& str) : std::string(str) {}
	string(std::string&& str) : std::string(std::move(str)) {}

	virtual bool is_string() const { return true; }

	std::size_t size() const noexcept { return string_base::size(); }

	string_base::reference at(string_base::size_type pos) { return string_base::at(pos); }
	string_base::const_reference at(string_base::size_type pos) const { return string_base::at(pos); }
	string_base::reference operator [](string_base::size_type pos) { return string_base::operator[](pos); }
	string_base::const_reference operator [](string_base::size_type pos) const { return string_base::operator[](pos); }

	virtual std::string str() const { return static_cast<std::string>(*this); }
	virtual std::string json_str() const { // in JSON format.
		std::ostringstream buf;
		buf << *this;
		return buf.str();
	}

	using value::pack;
	virtual std::vector<uint8_t>& pack(std::vector<uint8_t>& packed) const {
		std::size_t len = length();
		if (len <= 0x1f) {
			packed.push_back(static_cast<uint8_t>(0xa0 | len));
			packed.insert(packed.end(), cbegin(), cend());
		}
		else if (len <= 0xff) {
			packed.push_back(0xd9);
			packed.push_back(static_cast<uint8_t>(len));
			packed.insert(packed.end(), cbegin(), cend());
		}
		else if (len <= 0xffff) {
			packed.push_back(0xda);
			packed.push_back(static_cast<uint8_t>((len >> 8) & 0xff));
			packed.push_back(static_cast<uint8_t>(len & 0xff));
			packed.insert(packed.end(), cbegin(), cend());
		}
		else {
			packed.push_back(0xdb);
			packed.push_back(static_cast<uint8_t>((len >> 24) & 0xff));
			packed.push_back(static_cast<uint8_t>((len >> 16) & 0xff));
			packed.push_back(static_cast<uint8_t>((len >> 8) & 0xff));
			packed.push_back(static_cast<uint8_t>(len & 0xff));
			packed.insert(packed.end(), cbegin(), cend());
		}
		return packed;
	};
	virtual util::streambuf& pack(util::streambuf& packed) const {
		std::size_t len = length();
		if (len <= 0x1f) {
			packed.write_int8(static_cast<uint8_t>(0xa0 | len));
			std::memcpy(packed.prepare(len), &((*this)[0]), len);
			packed.commit(len);
		}
		else if (len <= 0xff) {
			packed.write_int8(static_cast<uint8_t>(0xd9));
			packed.write_int8(static_cast<uint8_t>(len));
			std::memcpy(packed.prepare(len), &((*this)[0]), len);
			packed.commit(len);
		}
		else if (len <= 0xffff) {
			packed.write_int8(static_cast<uint8_t>(0xda));
			packed.write_int8(static_cast<uint8_t>((len >> 8) & 0xff));
			packed.write_int8(static_cast<uint8_t>(len & 0xff));
			std::memcpy(packed.prepare(len), &((*this)[0]), len);
			packed.commit(len);
		}
		else {
			packed.write_int8(static_cast<uint8_t>(0xdb));
			packed.write_int8(static_cast<uint8_t>((len >> 24) & 0xff));
			packed.write_int8(static_cast<uint8_t>((len >> 16) & 0xff));
			packed.write_int8(static_cast<uint8_t>((len >> 8) & 0xff));
			packed.write_int8(static_cast<uint8_t>(len & 0xff));
			std::memcpy(packed.prepare(len), &((*this)[0]), len);
			packed.commit(len);
		}
		return packed;
	}

	using value::unpack;
	virtual const uint8_t* unpack(const uint8_t* bytes) {
		auto it = bytes;

		std::size_t len;
		if ((*it >= 0xa0) && (*it <= 0xbf)) {
			len = (*it & 0x1f);
		}
		else if (*it == 0xd9) {
			len = *(++it);
		}
		else if (*it == 0xda) {
			len = *(++it);
			len = (len << 8) + *(++it);
		}
		else if (*it == 0xdb) {
			len = *(++it);
			len = (len << 8) + *(++it);
			len = (len << 8) + *(++it);
			len = (len << 8) + *(++it);
		}
		else
			throw bad_format_error(std::string("Invalid MessagePack format."));

		it++;
		
		clear(); // make me empty.
		insert(end(), it, it + len);

		it += len;
		return it;
	}

	virtual const char* parse(const char* json_text) {
		auto it = json_text;

		__skip_sp(it);

		// "
		if (*it == '"')
			it++;
		else
			goto INVALID_FORMAT;

		while (true) {
			if (*it == '\\') {
				std::string unicode;
				char16_t utf16;

				it++;
				switch (*it) {
					case '"':
						push_back('"');
						it++;
						break;
					case '\\':
						push_back('\\');
						it++;
						break;
					case '/':
						push_back('/');
						it++;
						break;
					case 'b':
						push_back('\b');
						it++;
						break;
					case 'f':
						push_back('\f');
						it++;
						break;
					case 'n':
						push_back('\n');
						it++;
						break;
					case 'r':
						push_back('\r');
						it++;
						break;
					case 't':
						push_back('\t');
						it++;
						break;
					case 'u':
						it++;

						for (auto inx = 0 ; inx < 4 ; inx++) {
							if (std::isxdigit(*it)) {
								unicode.push_back(*it);
								it++;
							}
							else {
								goto INVALID_FORMAT;
							}
						}

						utf16 = static_cast<char16_t>(strtol(unicode.c_str(), nullptr, 16));
						if (unicode::is_lead_surrogate(utf16)) {
							// is next trail surrogate?
							if ((*it == '\\') && (*(it + 1) == 'u')) {
								it += 2;
								unicode.clear();
								for (auto inx = 0 ; inx < 4 ; inx++) {
									if (std::isxdigit(*it)) {
										unicode.push_back(*it);
										it++;
									}
									else {
										goto INVALID_FORMAT;
									}
								}

								char16_t trail_surrogate = static_cast<char16_t>(strtol(unicode.c_str(), nullptr, 16));

								if (unicode::is_trail_surrogate(trail_surrogate)) {
									append(unicode::utf32_to_utf8(unicode::surrogate_pair_to_utf32(utf16, trail_surrogate)));
								}
								else { // this case should be an error.
									append(unicode::utf16_to_utf8(utf16));
									append(unicode::utf16_to_utf8(trail_surrogate));
								}
							}
							else { // no surrogate. just append.
								append(unicode::utf16_to_utf8(utf16));
							}
						}
						else { // not surrogate. just append.
							append(unicode::utf16_to_utf8(utf16));
						}
						break;

					default:
						goto INVALID_FORMAT;
				}

			}
			else if (*it == '"') {
				it++;
				break;
			}
			else {
				push_back(*it);
				it++;
			}
		}

		return it;

	INVALID_FORMAT:
		if (*it != '\0')
			throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
		else
			throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
	}

	virtual void parse(std::istream& is) {
		int ch;

		ch = __skip_sp(is);

		// "
		if (ch != '"')
			goto INVALID_FORMAT;

		while (true) {
			ch = is.get();
			if (ch == '\\') {
				std::string unicode;
				char16_t utf16;

				ch = is.get();
				switch (ch) {
					case '"':
						push_back('"');
						break;
					case '\\':
						push_back('\\');
						break;
					case '/':
						push_back('/');
						break;
					case 'b':
						push_back('\b');
						break;
					case 'f':
						push_back('\f');
						break;
					case 'n':
						push_back('\n');
						break;
					case 'r':
						push_back('\r');
						break;
					case 't':
						push_back('\t');
						break;
					case 'u':
						for (auto inx = 0 ; inx < 4 ; inx++) {
							ch = is.get();
							if (std::isxdigit(ch))
								unicode.push_back(ch);
							else
								goto INVALID_FORMAT;
						}

						utf16 = static_cast<char16_t>(strtol(unicode.c_str(), nullptr, 16));
						if (unicode::is_lead_surrogate(utf16)) {
							// is next trail surrogate?
							ch = is.get();
							if ((ch == '\\') && (is.peek() == 'u')) {
								is.get(); // discard 'u'
								unicode.clear();
								for (auto inx = 0 ; inx < 4 ; inx++) {
									ch = is.get();
									if (std::isxdigit(ch))
										unicode.push_back(ch);
									else
										goto INVALID_FORMAT;
								}

								char16_t trail_surrogate = static_cast<char16_t>(strtol(unicode.c_str(), nullptr, 16));

								if (unicode::is_trail_surrogate(trail_surrogate)) {
									append(unicode::utf32_to_utf8(unicode::surrogate_pair_to_utf32(utf16, trail_surrogate)));
								}
								else {
									append(unicode::utf16_to_utf8(utf16));
									append(unicode::utf16_to_utf8(trail_surrogate));
								}
							}
							else { // no surrogate. just append.
								append(unicode::utf16_to_utf8(utf16));
								is.unget();
							}
						}
						else { // not surrogate. just append.
							append(unicode::utf16_to_utf8(utf16));
						}
						break;

					default:
						goto INVALID_FORMAT;
				}

			}
			else if (ch == std::istream::traits_type::eof()) {
				goto INVALID_FORMAT;
			}
			else if (ch == '"') {
				return;
			}
			else {
				push_back(ch);
			}
		}

	INVALID_FORMAT:
		if (ch == std::istream::traits_type::eof())
			throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
		else
			throw bad_format_error(std::string("Invalid JSON format: '") + (char)ch + "'");
	}

	friend std::ostream& operator<<(std::ostream& os, const string& str);
	friend std::istream& operator>>(std::istream& is, string& str);
};

inline bool operator==(const string& lhs, const string& rhs) noexcept {
	return (static_cast<std::string>(lhs) == static_cast<std::string>(rhs));
}

inline bool operator==(const char* lhs, const string& rhs) noexcept {
	return (static_cast<std::string>(rhs) == lhs);
}

inline bool operator==(const string& lhs, const char* rhs) noexcept {
	return (static_cast<std::string>(lhs) == rhs);
}

inline bool operator==(const std::string& lhs, const string& rhs) noexcept {
	return (static_cast<std::string>(rhs) == lhs);
}

inline bool operator==(const string& lhs, const std::string& rhs) noexcept {
	return (static_cast<std::string>(lhs) == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const string& str) {
	os << "\"";

	for (auto it = str.cbegin() ; it != str.cend() ; it++) {
		if (*it == '"')
			os << "\\\"";
		else if (*it == '\\')
			os << "\\\\";
		else if (*it == '/')
			os << "\\/";
		else if (*it == '\b')
			os << "\\b";
		else if (*it == '\f')
			os << "\\f";
		else if (*it == '\n')
			os << "\\n";
		else if (*it == '\r')
			os << "\\r";
		else if (*it == '\t')
			os << "\\t";
		else if ((*it >= 0x00) && (*it <= 0x1f)) { // control characters except for the above characters
			char buf[8];
			sprintf(buf, "\\u%04X", (int)*it);
			os << buf;
		}
		else {
			os << *it;
		}
	}

	return (os << "\"");
}

inline std::istream& operator>>(std::istream& is, string& str) {
	str.parse(is);
	return is;
}

} // namespace json
} // namespace shan

#endif /* shan_json_string_h */
