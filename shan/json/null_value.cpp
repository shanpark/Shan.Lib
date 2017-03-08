//
//  null_value.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "null_value.h"
#include "exception.h"

using namespace shan::json;

const char* null_value::parse(const char* json_text) {
	auto it = json_text;

	if (*it == 'n')
		it++;
	else
		goto INVALID_FORMAT;

	if (*it == 'u')
		it++;
	else
		goto INVALID_FORMAT;

	if (*it == 'l')
		it++;
	else
		goto INVALID_FORMAT;

	if (*it == 'l')
		it++;
	else
		goto INVALID_FORMAT;

	return it;

INVALID_FORMAT:
	if (static_cast<bool>(*it))
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
	else
		throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
}

void null_value::parse(std::istream& is) {
	int ch;

	if ((ch = is.get()) != 'n')
		goto INVALID_FORMAT;
	if ((ch = is.get()) != 'u')
		goto INVALID_FORMAT;
	if ((ch = is.get()) != 'l')
		goto INVALID_FORMAT;
	if ((ch = is.get()) != 'l')
		goto INVALID_FORMAT;
	return;

INVALID_FORMAT:
	if (ch == std::istream::traits_type::eof())
		throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + (char)ch + "'");
}

std::ostream& shan::json::operator<<(std::ostream& os, const null_value& n) {
	return (os << "null");
}

std::istream& shan::json::operator>>(std::istream& is, null_value& n) {
	n.parse(is);
	return is;
}
