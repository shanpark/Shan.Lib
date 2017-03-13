//
//  util.h
//  Shan.Util
//
//  Created by Sung Han Park on 2017. 3. 12..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_util_util_h
#define shan_util_util_h

#include <cstdint>

namespace shan {
namespace util {

static const union {
	uint32_t x;
	uint8_t c;
} __is_le{1};

inline uint16_t swap_bo16(uint16_t x) {
	return (((x & 0xff) << 8) | (x >> 8));
}
inline uint32_t swap_bo32(uint32_t x) {
	return (x << 24) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | (x >> 24);
}
inline uint64_t swap_bo64(uint64_t x) {
	return ((x << 56) | ((x << 40) & 0xff000000000000) | ((x << 24) & 0xff0000000000) | ((x << 8) & 0xff00000000) |
			((x >> 8) & 0xff000000) | ((x >> 24) & 0xff0000) | ((x >> 40) & 0xff00) | (x >> 56));
}

inline uint16_t hton16(uint16_t x) {
	return  ((__is_le.c) ? swap_bo16(x) : x);
}
inline uint16_t ntoh16(uint16_t x) {
	return hton16(x);
}
inline uint32_t hton32(uint32_t x) {
	return  ((__is_le.c) ? swap_bo32(x) : x);
}
inline uint32_t ntoh32(uint32_t x) {
	return hton32(x);
}
inline uint64_t hton64(uint64_t x) {
	// There is no perfect potable way to check endianness at compile time.
	// There is no standard function like htonll() for 64bit unsigned int.
	return  ((__is_le.c) ? swap_bo64(x) : x);
}
inline uint64_t ntoh64(uint64_t x) {
	return hton64(x);
}

} // namespace util
} // namespace shan

#endif /* shan_util_util_h */
