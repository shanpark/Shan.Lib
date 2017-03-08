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

using namespace shan::json;

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
		goto INVALID_FORMAT;
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
			goto INVALID_FORMAT;
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
			goto INVALID_FORMAT;
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

INVALID_FORMAT:
	if (static_cast<bool>(*it))
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
	else
		throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
}

void number::parse(std::istream& is) {
	bool integral = true;
	std::string num;
	int ch;

	if ((ch = skip_space(is)) == '-') {
		num.push_back(ch);
		ch = is.get();
	}

	if (isdigit(ch))
		num.push_back(ch);
	else
		goto INVALID_FORMAT;

	while (isdigit(ch = is.get()))
		num.push_back(ch);

	if (ch == '.') {
		num.push_back(ch);
		integral = false;

		if (isdigit(ch = is.get()))
			num.push_back(ch);
		else
			goto INVALID_FORMAT;

		while (isdigit(ch = is.get()))
			num.push_back(ch);
	}

	if ((ch == 'e') || (ch == 'E')) {
		num.push_back('e');
		integral = false;

		ch = is.get();
		if ((ch == '-') || (ch == '+')) {
			num.push_back(ch);
			ch = is.get();
		}

		if (isdigit(ch))
			num.push_back(ch);
		else
			goto INVALID_FORMAT;

		while (isdigit(ch = is.get()))
			num.push_back(ch);
	}

	is.unget(); // last ch should be returned to stream

	_integral = integral;
	_str_rep = num;
	if (_integral)
		_val._int = strtoll(_str_rep.c_str(), nullptr, 10);
	else
		_val._real = strtod(_str_rep.c_str(), nullptr);
	return;

INVALID_FORMAT:
	if (ch == std::istream::traits_type::eof())
		throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + (char)ch + "'");
}

std::ostream& shan::json::operator<<(std::ostream& os, const number& num) {
	return (os << num._str_rep);
}

std::istream& shan::json::operator>>(std::istream& is, number& num) {
	num.parse(is);
	return is;
}
