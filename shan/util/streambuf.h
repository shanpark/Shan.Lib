//
//  streambuf.h
//  Shan.Util
//
//  Created by Sung Han Park on 2017. 3. 18..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_util_streambuf_h
#define shan_util_streambuf_h

#include <cstring>
#include "exception.h"
#include "util.h"
#include "pool.h"

namespace shan {
namespace util {

class streambuf : public shan::object, public std::streambuf, public noncopyable, public poolable {
public:
	explicit streambuf(std::size_t base_size = __def_size)
	: _base_size(base_size), _buffer() {
		_base_size = std::max<std::size_t>(_base_size, __min_size);
		_buffer.resize(_base_size);
		setg(&_buffer[0], &_buffer[0], &_buffer[0]);
		setp(&_buffer[0], &_buffer[0] + _base_size);
	}

	// poolable interface. (non-virtual)
	virtual void reset(std::size_t base_size = __def_size) noexcept { // parameters must be the same as the constructor.
		setg(&_buffer[0], &_buffer[0], &_buffer[0]);
		setp(&_buffer[0], &_buffer[0] + _buffer.size());
	}

	// Get the base size
	std::size_t base_size() const {
		return _base_size;
	}

	// Get the size of the memory allocated for use as a buffer.
	std::size_t alloc_size() const {
		return _buffer.size();
	}

	// Get the size of the input sequence.
	std::size_t in_size() const {
		return pptr() - gptr();
	}

	// Get the pointer of the input sequence
	const char* in_ptr() const {
		return gptr();
	}

	// Get the size of the output sequence.
	std::size_t out_size() const {
		return epptr() - pptr();
	}

	// Get the pointer of the output sequence
	const char* out_ptr() const {
		return pptr();
	}

	// Reserve n sized buffer & Get the pointer of the output sequence
	// if possible, prepare() can reduce allocated memory size.
	char* prepare(std::size_t n) {
		reserve(n);
		return pptr();
	}

	// Move characters from the output sequence to the input sequence.
	void commit(std::size_t n) {
		if (pptr() + n > epptr())
			n = epptr() - pptr();
		pbump(static_cast<int>(n));
		setg(eback(), gptr(), pptr());
	}

	// Remove characters from the input sequence.
	void consume(std::size_t n) {
		if (egptr() < pptr())
			setg(&_buffer[0], gptr(), pptr());
		if (gptr() + n > pptr())
			n = pptr() - gptr();
		gbump(static_cast<int>(n));
	}

	void append(streambuf& other) {
		auto size = other.in_size();
		reserve(size);
		std::memcpy(pptr(), other.in_ptr(), size);
		commit(size);
		other.consume(other.in_size());
	}
	
	std::size_t read(void* buf, std::size_t size_in_byte) {
		auto size = std::min<std::size_t>(in_size(), size_in_byte);
		std::memcpy(buf, gptr(), size);
		consume(size);
		return size;
	}

	std::size_t read_int8(int8_t* val) {
		if (in_size() < 1)
			return 0;
		return read(val, 1);
	}
	std::size_t read_int16(int16_t* val) {
		if (in_size() < 2)
			return 0;
		return read(val, 2);
	}
	std::size_t read_int32(int32_t* val) {
		if (in_size() < 4)
			return 0;
		return read(val, 4);
	}
	std::size_t read_int64(int64_t* val) {
		if (in_size() < 8)
			return 0;
		return read(val, 8);
	}

	std::size_t read_float(float* val) {
		if (in_size() < 4)
			return 0;
		return read(&val, 4);
	}
	std::size_t read_double(double* val) {
		if (in_size() < 8)
			return 0;
		return read(&val, 8);
	}

	std::size_t write(const void* buf, std::size_t size_in_byte) {
		reserve(size_in_byte);
		std::memcpy(pptr(), buf, size_in_byte);
		pbump(static_cast<int>(size_in_byte));
		return size_in_byte;
	}

	std::size_t write_int8(int8_t val) {
		return write(&val, 1);
	}
	std::size_t write_int16(int16_t val) {
		return write(&val, 2);
	}
	std::size_t write_int32(int32_t val) {
		return write(&val, 4);
	}
	std::size_t write_int64(int64_t val) {
		return write(&val, 8);
	}

	std::size_t write_float(float val) {
		return write(&val, 4);
	}
	std::size_t write_double(double val) {
		return write(&val, 8);
	}

	std::size_t write_string(const char* str) {
		std::size_t size = std::strlen(str);
		return write(&str, size);
	}
	std::size_t write_string(const std::string& str) {
		std::size_t size = str.length();
		return write(str.c_str(), size);
	}

protected:
	// Override std::streambuf behaviour.
	int_type underflow() {
		if (gptr() < pptr()) {
			setg(&_buffer[0], gptr(), pptr());
			return traits_type::to_int_type(*gptr());
		}
		else {
			return traits_type::eof();
		}
	}

	// Override std::streambuf behaviour.
	int_type overflow(int_type c) {
		if (!traits_type::eq_int_type(c, traits_type::eof())) {
			if (pptr() == epptr()) // No space to write a char
				reserve(1);

			*pptr() = traits_type::to_char_type(c);
			pbump(1);
			return c;
		}

		return traits_type::not_eof(c);
	}

	void reserve(std::size_t n) {
		// Get current stream positions as offsets.
		std::size_t gnext = gptr() - &_buffer[0];
		std::size_t pnext = pptr() - &_buffer[0];
		std::size_t pend = epptr() - &_buffer[0];

		// Check if there is already enough space in the put area.
		if (n <= pend - pnext)
			return;

		// Shift existing contents of get area to start of buffer.
		if (gnext > 0) {
			pnext -= gnext;
			std::memmove(&_buffer[0], &_buffer[0] + gnext, pnext);
		}

		// Ensure buffer is large enough to hold at least the specified size.
		if (n > pend - pnext) {
			auto limit = std::numeric_limits<std::size_t>::max() >> 1;
			while ((n > pend - pnext) && (pend <= limit)) // doubles buffer size.
				pend <<= 1;

			if (n > pend - pnext) // still not enough
				throw buffer_error("too big buffer requested");
			else
				_buffer.resize((std::max<std::size_t>)(pend, 1));
		}
		else {
			if (pend > (_base_size << 1)) { // too much capacity..
				if (pnext + n < (pend >> 1)) { // if possible, reduce to half
					pend >>= 1;
					std::vector<char> tmp_buf(pend);
					std::memcpy(&tmp_buf[0], &_buffer[0], pnext);
					_buffer = std::move(tmp_buf);
				}
			}
		}

		// Update stream positions.
		setg(&_buffer[0], &_buffer[0], &_buffer[0] + pnext);
		setp(&_buffer[0] + pnext, &_buffer[0] + pend);
	}

	enum { __min_size = 128, __def_size = 4096 };

protected:
	std::size_t _base_size;
	std::vector<char> _buffer;
};

using streambuf_ptr = std::shared_ptr<streambuf>;

} // namespace util
} // namespace shan

#endif /* shan_util_streambuf_h */
