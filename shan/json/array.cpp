//
//  array.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "array.h"
#include "object.h"
#include "string.h"
#include "true_value.h"
#include "false_value.h"
#include "null_value.h"
#include "number.h"
#include "exception.h"

using namespace shan::json;

//namespace shan {
//namespace json {

array::array(const char* arr_str) {
	array::parse(arr_str);
}

const char* array::parse(const char* json_text) {
	auto it = json_text;

	skip_space(it);

	// [
	if (*it == '[')
		skip_space(++it);
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");

	if (*it == ']') // empty array
		return ++it;

	while (true) {
		// value
		std::shared_ptr<value> val;
		switch (*it) {
			case '{':
				val = std::make_shared<shan::json::object>();
				break;
			case '[':
				val = std::make_shared<array>();
				break;
			case '"':
				val = std::make_shared<string>();
				break;
			case 't':
				val = std::make_shared<true_value>();
				break;
			case 'f':
				val = std::make_shared<false_value>();
				break;
			case 'n':
				val = std::make_shared<null_value>();
				break;
			default:
				if ((*it == '-') || (isdigit(*it)))
					val = std::make_shared<number>();
				else
					throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
				break;
		}
		it = val->parse(it);

		// push value
		push_back(val);

		skip_space(it);

		if (*it == ',')
			skip_space(++it);
		else if (*it == ']')
			break;
		else
			throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
	}

	return ++it;
}

std::string array::str() const {
	std::stringstream buf;
	buf << *this;
	return buf.str();
}

std::ostream& shan::json::operator<<(std::ostream& os, const array& arr) {
	os << "[";

	for (auto it = arr.cbegin() ; it != arr.cend() ; ) {
//		auto& ti = typeid(*it); // replaced by next 2 lines because of a warning. (Is this compiler bug?)
		auto temp = it->get();
		auto& ti = typeid(*temp);
		if (ti == typeid(object))
			os << *(std::static_pointer_cast<object>(*it));
		else if (ti == typeid(array))
			os << *(std::static_pointer_cast<array>(*it));
		else if (ti == typeid(string))
			os << *(std::static_pointer_cast<string>(*it));
		else if (ti == typeid(number))
			os << *(std::static_pointer_cast<number>(*it));
		else if (ti == typeid(true_value))
			os << *(std::static_pointer_cast<true_value>(*it));
		else if (ti == typeid(false_value))
			os << *(std::static_pointer_cast<false_value>(*it));
		else if (ti == typeid(null_value))
			os << *(std::static_pointer_cast<null_value>(*it));

		it++;
		if (it == arr.cend())
			break;

		os << ',';
	}

	return (os << "]");
}

//}
//}
