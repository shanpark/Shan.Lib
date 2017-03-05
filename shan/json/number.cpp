//
//  number.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <cctype>
#include "number.h"
#include "exception.h"

namespace shan {
namespace json {

number::number(const char* num_str) {
	number::parse(num_str);
}

const char* number::parse(const char* json_text) {
	auto it = json_text;

	bool integral = true;
	std::string num;

	skip_space(it);

	if (*it == '-') {
		num.push_back(*it);
		it++;
	}

	if (isdigit(*it)) {
		num.push_back(*it);
		it++;
	}
	else {
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
	}

	while (isdigit(*it)) {
		num.push_back(*it);
		it++;
	}

	if (*it == '.') {
		num.push_back(*it);
		it++;
		integral = false;

		if (isdigit(*it)) {
			num.push_back(*it);
			it++;
		}
		else {
			throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
		}

		while (isdigit(*it)) {
			num.push_back(*it);
			it++;
		}
	}

	if ((*it == 'e') || (*it == 'E')) {
		num.push_back('e');
		it++;
		integral = false;

		if ((*it == '-') || (*it == '+')) {
			num.push_back(*it);
			it++;
		}

		if (isdigit(*it)) {
			num.push_back(*it);
			it++;
		}
		else {
			throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
		}

		while (isdigit(*it)) {
			num.push_back(*it);
			it++;
		}
	}

	_integral = integral;
	_str_rep = num;
	if (_integral)
		_val._int = strtoll(_str_rep.c_str(), nullptr, 10);
	else
		_val._real = strtod(_str_rep.c_str(), nullptr);

	return it;
}

int64_t number::int_val() {
	return _integral ? _val._int : (int64_t)_val._real;
}

double number::real_val() {
	return _integral ? (double)_val._int : _val._real;
}

std::ostream& operator<<(std::ostream& os, const number& num) {
	return (os << num._str_rep);
}

}
}
