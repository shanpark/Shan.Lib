//
//  false_value.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "false_value.h"
#include "exception.h"

namespace shan {
namespace json {

const char* false_value::parse(const char* json_text) {
	auto it = json_text;

	if (*it == 'f')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 'a')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 'l')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 's')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 'e')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	return it;
}

std::ostream& operator<<(std::ostream& os, const false_value& f) {
	return (os << "false");
}

}
}
