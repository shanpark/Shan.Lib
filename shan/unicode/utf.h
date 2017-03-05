/*

  utf.h
  Shan.Unicode

 Copyright (c) 2017, Sung-Han Park
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef shan_unicode_utf_h
#define shan_unicode_utf_h

#include <string>

namespace shan {
namespace unicode {

bool is_lead_surrogate(char16_t utf16) {
	return ((utf16 & 0xFC00) == 0xD800);
}

bool is_trail_surrogate(char16_t utf16) {
	return ((utf16 & 0xFC00) == 0xDC00);
}

char32_t surrogate_pair_to_utf32(char16_t lead, char16_t trail) {
	return (((lead & 0x03FF) << 10) + (trail & 0x03FF)) + 0x00010000;
}

std::u16string utf32_to_surrogate_pair(char32_t utf32) {
	std::string ret;
	char16_t high, low;

	if (utf32 <= 0xffff) { // no surrogate char.
		ret.push_back(utf32);
	}
	else {
		high = (((((utf32 >> 16) - 1) & 0x0f) << 6) | ((utf32 >> 10) & 0x3f)) | 0xdc00;
		low = ((utf32 & 0x03ff) | 0xd800);

		ret.push_back(high);
		ret.push_back(low);
	}

	return ret;
}

std::string to_utf8(char16_t utfnn) {
	return to_utf8((char32_t)utfnn);
}

std::string to_utf8(char32_t utfnn) {
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

char32_t to_utf32(const std::string& utf8, int* used) {
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

}
}

#endif /* shan_unicode_utf_h */
