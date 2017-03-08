//
//  array.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <cctype>
#include <sstream>
#include "object.h"
#include "array.h"
#include "string.h"
#include "number.h"
#include "true_value.h"
#include "false_value.h"
#include "null_value.h"
#include "exception.h"

using namespace shan::json;

void array::add(const shan::json::object& obj) {
	push_back(std::make_shared<shan::json::object>(obj));
}

void array::add(shan::json::object&& obj) {
	push_back(std::make_shared<shan::json::object>(std::move(obj)));
}

std::string array::str() const {
	std::stringstream buf;
	buf << *this;
	return buf.str();
}

const char* array::parse(const char* json_text) {
	auto it = json_text;

	skip_space(it);

	// [
	if (*it == '[')
		skip_space(++it);
	else
		goto INVALID_FORMAT;

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
					goto INVALID_FORMAT;
				break;
		}
		it = val->parse(it);

		// push value
		push_back(std::move(val));

		skip_space(it);

		if (*it == ',')
			skip_space(++it);
		else if (*it == ']')
			break;
		else
			goto INVALID_FORMAT;
	}

	return ++it;

INVALID_FORMAT:
	if (static_cast<bool>(*it))
		throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
	else
		throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
}

void array::parse(std::istream& is) {
	int ch;

	// [
	if ((ch = skip_space(is)) != '[')
		goto INVALID_FORMAT;

	if ((ch = skip_space(is)) == ']') // empty array
		return;

	while (true) {
		// value
		std::shared_ptr<value> val;
		switch (ch) {
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
				if ((ch == '-') || (isdigit(ch)))
					val = std::make_shared<number>();
				else
					goto INVALID_FORMAT;
				break;
		}
		is.unget();
		val->parse(is);

		// push value
		push_back(std::move(val));

		ch = skip_space(is);
		if (ch == ',')
			ch = skip_space(is);
		else if (ch == ']')
			return;
		else
			goto INVALID_FORMAT;
	}

INVALID_FORMAT:
	if (ch == std::istream::traits_type::eof())
		throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
	else
		throw bad_format_error(std::string("Invalid JSON format: '") + (char)ch + "'");
}

std::ostream& shan::json::operator<<(std::ostream& os, const array& arr) {
	os << "[";

	for (auto it = arr.cbegin() ; it != arr.cend() ; ) {
//		auto& ti = typeid(*it); // replaced by next 2 lines because of a warning. (Is this compiler bug?)
		auto temp = it->get();
		auto& ti = typeid(*temp);
		if (ti == typeid(shan::json::object))
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

std::istream& shan::json::operator>>(std::istream& is, array& arr) {
	arr.parse(is);
	return is;
}
