//
//  true_value.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#include "true_value.h"
#include "exception.h"

namespace shan {
namespace json {

const char* true_value::parse(const char* json_text) {
	auto it = json_text;

	if (*it == 't')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 'r')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 'u')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == 'e')
		it++;
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	return it;
}

std::ostream& operator<<(std::ostream& os, const true_value& t) {
	return (os << "true");
}

}
}
