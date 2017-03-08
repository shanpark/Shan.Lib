//
//  string.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <cctype>
#include <sstream>
#include "string.h"
#include "exception.h"
#include "unicode/utf.h"

using namespace shan::json;

std::string string::json_str() const { // in JSON format.
	std::stringstream buf;
	buf << *this;
	return buf.str();
}

const char* string::parse(const char* json_text) {
	auto it = json_text;

	skip_space(it);

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
						if (ishexnumber(*it)) {
							unicode.push_back(*it);
							it++;
						}
						else {
							goto INVALID_FORMAT;
						}
					}

					utf16 = strtol(unicode.c_str(), nullptr, 16); // utf16 -> utf8

					if (unicode::is_lead_surrogate(utf16)) {
						// is next trail surrogate?
						if ((*it == '\\') && (*(it + 1) == 'u')) {
							it += 2;
							unicode.clear();
							for (auto inx = 0 ; inx < 4 ; inx++) {
								if (ishexnumber(*it)) {
									unicode.push_back(*it);
									it++;
								}
								else {
									goto INVALID_FORMAT;
								}
							}

							char16_t trail_surrogate = strtol(unicode.c_str(), nullptr, 16); // utf16 -> utf8
							
							if (unicode::is_trail_surrogate(trail_surrogate)) {
								append(unicode::to_utf8(unicode::surrogate_pair_to_utf32(utf16, trail_surrogate)));
							}
							else {
								append(unicode::to_utf8(utf16));
								append(unicode::to_utf8(trail_surrogate));
							}
						}
						else { // no surrogate. just append.
							append(unicode::to_utf8(utf16));
						}
					}
					else { // not surrogate. just append.
						append(unicode::to_utf8(utf16));
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
	if (static_cast<bool>(*it))
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
	else
		throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
}

void string::parse(std::istream& is) {
	int ch;

	ch = skip_space(is);

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
						if (ishexnumber(ch))
							unicode.push_back(ch);
						else
							goto INVALID_FORMAT;
					}

					utf16 = strtol(unicode.c_str(), nullptr, 16); // utf16 -> utf8

					if (unicode::is_lead_surrogate(utf16)) {
						// is next trail surrogate?
						ch = is.get();
						if ((ch == '\\') && (is.peek() == 'u')) {
							is.get(); // discard 'u'
							unicode.clear();
							for (auto inx = 0 ; inx < 4 ; inx++) {
								ch = is.get();
								if (ishexnumber(ch))
									unicode.push_back(ch);
								else
									goto INVALID_FORMAT;
							}

							char16_t trail_surrogate = strtol(unicode.c_str(), nullptr, 16); // utf16 -> utf8

							if (unicode::is_trail_surrogate(trail_surrogate)) {
								append(unicode::to_utf8(unicode::surrogate_pair_to_utf32(utf16, trail_surrogate)));
							}
							else {
								append(unicode::to_utf8(utf16));
								append(unicode::to_utf8(trail_surrogate));
							}
						}
						else { // no surrogate. just append.
							append(unicode::to_utf8(utf16));
							is.unget();
						}
					}
					else { // not surrogate. just append.
						append(unicode::to_utf8(utf16));
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

bool operator==(const string& lhs, const string& rhs) noexcept {
	return (static_cast<std::string>(lhs) == static_cast<std::string>(rhs));
}

bool operator==(const char* lhs, const string& rhs) noexcept {
	return (static_cast<std::string>(rhs) == lhs);
}

bool operator==(const string& lhs, const char* rhs) noexcept {
	return (static_cast<std::string>(lhs) == rhs);
}

bool operator==(const std::string& lhs, const string& rhs) noexcept {
	return (static_cast<std::string>(rhs) == lhs);
}

bool operator==(const string& lhs, const std::string& rhs) noexcept {
	return (static_cast<std::string>(lhs) == rhs);
}

std::ostream& shan::json::operator<<(std::ostream& os, const string& str) {
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
//		else if (*it & 0x80) { // any unicode characters can be placed in JSON string.
//			int used_bytes = 0;
//			char32_t utf32 = unicode::to_utf32(&(*it), &used_bytes);
//			if (used_bytes == 0) { // converting fail
//				os << *it;
//			}
//			else {
//				it += (used_bytes - 1);
//
//				char buf[8];
//				auto utf16 = unicode::utf32_to_utf16(utf32);
//				for (auto jt = utf16.cbegin() ; jt != utf16.cend() ; jt++) {
//					sprintf(buf, "\\u%04X", (int)*jt);
//					os << buf;
//				}
//			}
//		}
		else {
			os << *it;
		}
	}

	return (os << "\"");
}

std::istream& operator>>(std::istream& is, string& str) {
	str.parse(is);
	return is;
}
