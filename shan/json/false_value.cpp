//
//  false_value.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "false_value.h"
#include "exception.h"

using namespace shan::json;

const char* false_value::parse(const char* json_text) {
	auto it = json_text;

	if (*it == 'f')
		it++;
	else
		goto INVALID_FORMAT;

	if (*it == 'a')
		it++;
	else
		goto INVALID_FORMAT;

	if (*it == 'l')
		it++;
	else
		goto INVALID_FORMAT;

	if (*it == 's')
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

void false_value::parse(std::istream& is) {
	int ch;

	if ((ch = is.get()) != 'f')
		goto INVALID_FORMAT;
	if ((ch = is.get()) != 'a')
		goto INVALID_FORMAT;
	if ((ch = is.get()) != 'l')
		goto INVALID_FORMAT;
	if ((ch = is.get()) != 's')
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

std::ostream& shan::json::operator<<(std::ostream& os, const false_value& f) {
	return (os << "false");
}

std::istream& shan::json::operator>>(std::istream& is, false_value& f) {
	f.parse(is);
	return is;
}
