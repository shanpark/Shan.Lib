//
//  utf.h
//  Shan.Unicode
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_unicode_utf_h
#define shan_unicode_utf_h

#include <string>

namespace shan {
namespace unicode {

inline bool is_lead_surrogate(char16_t utf16) noexcept {
	return ((utf16 & 0xFC00) == 0xD800);
}

inline bool is_trail_surrogate(char16_t utf16) noexcept {
	return ((utf16 & 0xFC00) == 0xDC00);
}

inline char32_t surrogate_pair_to_utf32(char16_t lead, char16_t trail) noexcept {
	return (((lead & 0x03FF) << 10) + (trail & 0x03FF)) + 0x00010000;
}

inline std::u16string utf32_to_utf16(char32_t utf32) {
	std::u16string ret;
	char16_t lead, trail;

	if (utf32 <= 0xffff) { // no surrogate char.
		ret.push_back(utf32);
	}
	else { // make surrogate pair
		lead = 0xD800 | ((utf32 - 0x10000) >> 10);
		trail = 0xDC00 | (utf32 & 0x3FF);

		ret.push_back(lead);
		ret.push_back(trail);
	}

	return ret;
}

inline std::string utf32_to_utf8(char32_t utfnn) {
	std::string ret;

	if (utfnn < 0x80) {
		ret.push_back(static_cast<char>(utfnn));
	}
	else if (utfnn < 0x800) {
		ret.push_back(static_cast<char>((utfnn >> 6) | 0xc0));
		ret.push_back(static_cast<char>((utfnn & 0x3f) | 0x80));
	}
	else if (utfnn < 0x10000) {
		ret.push_back(static_cast<char>((utfnn >> 12) | 0xe0));
		ret.push_back(static_cast<char>(((utfnn >> 6) & 0x3f) | 0x80));
		ret.push_back(static_cast<char>((utfnn & 0x3f) | 0x80));
	}
	else if (utfnn < 0x200000) {
		ret.push_back(static_cast<char>((utfnn >> 18) | 0xf0));
		ret.push_back(static_cast<char>(((utfnn >> 12) & 0x3f) | 0x80));
		ret.push_back(static_cast<char>(((utfnn >> 6) & 0x3f) | 0x80));
		ret.push_back(static_cast<char>((utfnn & 0x3f) | 0x80));
	}
//	else if (utfnn < 0x4000000) { // is not required for current version of unicode.
//		ret.push_back(static_cast<char>((utfnn >> 24) | 0xf8));
//		ret.push_back(static_cast<char>(((utfnn >> 18) & 0x3f) | 0x80));
//		ret.push_back(static_cast<char>(((utfnn >> 12) & 0x3f) | 0x80));
//		ret.push_back(static_cast<char>(((utfnn >> 6) & 0x3f) | 0x80));
//		ret.push_back(static_cast<char>((utfnn & 0x3f) | 0x80));
//	}
//	else if (utfnn < 0x80000000) {
//		ret.push_back(static_cast<char>((utfnn >> 30) | 0xfc));
//		ret.push_back(static_cast<char>(((utfnn >> 24) & 0x3f) | 0x80));
//		ret.push_back(static_cast<char>(((utfnn >> 18) & 0x3f) | 0x80));
//		ret.push_back(static_cast<char>(((utfnn >> 12) & 0x3f) | 0x80));
//		ret.push_back(static_cast<char>(((utfnn >>  6) & 0x3f) | 0x80));
//		ret.push_back(static_cast<char>((utfnn & 0x3f) | 0x80));
//	}

	return ret;
}

inline std::string utf16_to_utf8(char16_t utfnn) {
	return utf32_to_utf8((char32_t)utfnn);
}

inline char32_t utf8_to_utf32(const char* utf8, int* used) noexcept {
	int used_byte = 0;
	char32_t utf32 = 0;

	int first_byte = (utf8[used_byte] & 0xff); // to handle as unsigned.
	if (first_byte <= 0x7f) {
		utf32 = (utf8[used_byte++] & 0x7f);
	}
	else if (first_byte <= 0xdf) {
		utf32 = (utf8[used_byte++] & 0x1f);
		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
	}
	else if (first_byte <= 0xef) {
		utf32 = (utf8[used_byte++] & 0x0f);
		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
	}
	else if (first_byte <= 0xf7) {
		utf32 = (utf8[used_byte++] & 0x07);
		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
	}
//	else if (first_byte <= 0xfb) { // is not required for current version of unicode.
//		utf32 = (utf8[used_byte++] & 0x03);
//		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
//		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
//		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
//		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
//	}
//	else if (first_byte <= 0xfd) {
//		utf32 = (utf8[used_byte++] & 0x01);
//		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
//		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
//		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
//		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
//		utf32 = (utf32 << 6) | (utf8[used_byte++] & 0x3f);
//	}

	if (used)
		*used = used_byte;
	return utf32;
}

inline std::u16string utf8_to_utf16(const char* utf8, int* used) {
	return utf32_to_utf16(utf8_to_utf32(utf8, used));
}

}
}

#endif /* shan_unicode_utf_h */
