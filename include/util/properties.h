//
//  properties.h
//  Shan.Util
//
//  Created by Sung Han Park on 2017. 4. 29..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_util_properties_h
#define shan_util_properties_h

#include <map>
#include "../object.h"
#include "unicode/utf.h"
#include "exception.h"

namespace shan {
namespace util {

class properties : public object {
public:
	void load(std::istream& is) {
		int ch;
		std::string key, value;

		ch = is.get();
		while (ch != std::istream::traits_type::eof()) {
			if (std::isspace(ch)) {
				ch = __skip_sp(is); // key가 시작되기 전 white는 모두 넘긴다.
				continue;
			}
			else if ((ch == '#') || (ch == '!')) { // comments;
				ch = __skip_line(is);
				continue;
			}
			else { // key
				key = __read_key(ch, is);

				while (true) {
					if ((ch != ' ') && (ch != '\t') && (ch != '\f'))
						break;
					ch = is.get();
				}

				if ((ch == '=') || (ch == ':'))
					ch = is.get(); // discard

				while (true) {
					if ((ch != ' ') && (ch != '\t') && (ch != '\f'))
						break;
					ch = is.get();
				}

				value = __read_value(ch, is);

				if (key.length() > 0)
					_properties.insert(std::make_pair(key, value));
			}
		}
	}

	const std::string& get(const std::string& key) throw(not_found_error) {
		try {
			return _properties.at(key);
		} catch (const std::exception& e) {
			throw not_found_error(e.what());
		}
	}

protected:
	int __skip_sp(std::istream& is) {
		int ch;
		while (std::isspace(ch = is.get())) // skip spaces
			;
		return ch; // return first non-space char
	}

	int __skip_line(std::istream& is) {
		int ch;
		while (true) {
			ch = is.get();
			if ((ch == '\n') || (ch == '\r') || (ch == std::istream::traits_type::eof()))
				break;
		}
		return ch;
	}

	int __handle_backslash(std::string& str, std::istream& is) {
		int ch = is.get();
		if (ch == 't')
			str.push_back('\t');
		else if (ch == 'n')
			str.push_back('\n');
		else if (ch == 'f')
			str.push_back('\f');
		else if (ch == 'r')
			str.push_back('\r');
		else if (ch == '\"')
			str.push_back('\"');
		else if (ch == '\'')
			str.push_back('\'');
		else if (ch == '\\')
			str.push_back('\\');
		else if (ch == '\n') { // line terminator. 다음 줄 맨 앞 공백까지 넘겨버린다.
			while (true) {
				ch = is.get();
				if ((ch != ' ') && (ch != '\t') && (ch != '\f'))
					return ch;
			}
		}
		else if (ch == '\r') { // line terminator. 다음 줄 맨 앞 공백까지 넘겨버린다.
			ch = is.get();
			if (ch == '\n')
				ch = is.get();
			while (true) {
				ch = is.get();
				if ((ch != ' ') && (ch != '\t') && (ch != '\f'))
					return ch;
			}
		}
		else if (ch == 'u') { // unicode sequence
			std::string unicode;
			for (auto inx = 0 ; inx < 4 ; inx++) {
				ch = is.get();
				if (std::isxdigit(ch))
					unicode.push_back(ch);
				else {
					str.push_back('u');
					str.append(unicode);
					return ch;
				}
			}

			char16_t utf16 = static_cast<char16_t>(strtol(unicode.c_str(), nullptr, 16));
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
						else {
							str.append(unicode::utf16_to_utf8(utf16));

							str.push_back('u');
							str.append(unicode);
							return ch;
						}
					}

					char16_t trail_surrogate = static_cast<char16_t>(strtol(unicode.c_str(), nullptr, 16));
					if (unicode::is_trail_surrogate(trail_surrogate)) {
						str.append(unicode::utf32_to_utf8(unicode::surrogate_pair_to_utf32(utf16, trail_surrogate)));
					}
					else {
						str.append(unicode::utf16_to_utf8(utf16));
						str.append(unicode::utf16_to_utf8(trail_surrogate));
					}
				}
				else { // no surrogate. just append.
					str.append(unicode::utf16_to_utf8(utf16));
					return ch; // 이미 다음 문자를 읽어서 가지고 있으므로 바로 리턴한다.
				}
			}
			else { // not surrogate. just append.
				str.append(unicode::utf16_to_utf8(utf16));
			}
		}
		else if (ch == std::istream::traits_type::eof()) {
			return ch;
		}
		else {
			str.push_back(ch);
		}

		ch = is.get();
		return ch;
	}

	std::string __read_key(int& ch, std::istream& is) {
		std::string key;

		while (ch != std::istream::traits_type::eof()) {
			if (ch == '\\')
				ch = __handle_backslash(key, is); // backslash에서 생성되는 문자를 key에 넣어주고 그 다음 문자를 반환한다.
			else if ((ch == '=') || (ch == ':'))
				return key;
			else if (std::isspace(ch))
				return key;
			else {
				key.push_back(ch);
				ch = is.get();
			}
		}

		return key;
	}

	std::string __read_value(int& ch, std::istream& is) {
		std::string value;

		while (ch != std::istream::traits_type::eof()) {
			if (ch == '\\')
				ch = __handle_backslash(value, is); // backslash에서 생성되는 문자를 key에 넣어주고 그 다음 문자를 반환한다.
			else if ((ch == '\n') || (ch == '\r'))
				return value;
			else {
				value.push_back(ch);
				ch = is.get();
			}
		}

		return value;
	}

protected:
	std::map<std::string, std::string> _properties;
};

} // namespace util
} // namespace shan

#endif /* shan_util_properties_h */
