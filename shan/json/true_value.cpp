//
//  true_value.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "true_value.h"
#include "exception.h"

using namespace shan::json;

const char* true_value::parse(const char* json_text) {
	auto it = json_text;

	if (*it == 't')
		it++;
	else
		goto INVALID_FORMAT;

	if (*it == 'r')
		it++;
	else
		goto INVALID_FORMAT;

	if (*it == 'u')
		it++;
	else
		goto INVALID_FORMAT;

	if (*it == 'e')
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

void true_value::parse(std::istream& is) {
	int ch;

	if ((ch = is.get()) != 't')
		goto INVALID_FORMAT;
	if ((ch = is.get()) != 'r')
		goto INVALID_FORMAT;
	if ((ch = is.get()) != 'u')
		goto INVALID_FORMAT;
	if ((ch = is.get()) != 'e')
		goto INVALID_FORMAT;
	return;

INVALID_FORMAT:
	if (ch == std::istream::traits_type::eof())
		throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + (char)ch + "'");
}

std::ostream& shan::json::operator<<(std::ostream& os, const true_value& t) {
	return (os << "true");
}

std::istream& shan::json::operator>>(std::istream& is, true_value& t) {
	t.parse(is);
	return is;
}
