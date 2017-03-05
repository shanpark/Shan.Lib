//
//  null_value.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#include "null_value.h"
#include "exception.h"

namespace shan {
namespace json {

const char* null_value::parse(const char* json_text) {
	auto it = json_text;

	if (*it == 'n')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 'u')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 'l')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 'l')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	return it;
}

std::ostream& operator<<(std::ostream& os, const null_value& n) {
	return (os << "null");
}

}
}
