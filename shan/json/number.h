//
//  number.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_number_h
#define shan_json_number_h

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <type_traits>
#include "value.h"

namespace shan {
namespace json {

class number : public value {
public:
	number() : _integral(true) {}
	number(const char* num_str) { number::parse(num_str); }
	number(const std::string& num_str) { number::parse(num_str.c_str()); }
	template<typename T>
	number(T val) {
		static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "Parameter 'val' should be a number type.");
		_integral = std::is_integral<T>();
		if (_integral) {
			_val._int = static_cast<int64_t>(val);
			_str_rep = std::to_string(_val._int);
		}
		else {
			_val._real = static_cast<double>(val);
			_str_rep = double_to_string(_val._real);
		}
	}

	int64_t int_val() const { return _integral ? _val._int : static_cast<int64_t>(_val._real); }
	double real_val() const { return _integral ? static_cast<double>(_val._int) : _val._real; }

	virtual std::string str() const { return _str_rep; }

	virtual const char* parse(const char* json_text);
	virtual void parse(std::istream& is);

	friend std::ostream& operator<<(std::ostream& os, const number& num);
	friend std::istream& operator>>(std::istream& is, number& num);

protected:
	std::string double_to_string(double val) {
		char buf[64];
		std::sprintf(buf, "%.15f", val);
		std::string str(buf);
		auto pos = str.find_last_not_of('0');
		return ((str[pos] == '.') && (pos < str.length() - 1)) ? str.erase(pos + 2) : str.erase(pos + 1);
	}

protected:
	bool _integral;
	union val {
		val() : _int(0) {}
		val(int64_t val) : _int(val) {}
		val(double val) : _real(val) {}

		int64_t _int;
		double _real;
	} _val;
	std::string _str_rep;
};

std::ostream& operator<<(std::ostream& os, const number& num);
std::istream& operator>>(std::istream& is, number& num);

} // namespace json
} // namespace shan

#endif /* shan_json_number_h */
