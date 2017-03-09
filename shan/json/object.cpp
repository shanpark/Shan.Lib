//
//  object.cpp
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

value_ptr object::at(const object_base::key_type& key) {
	return object_base::at(key);
}

const_value_ptr object::at(const object_base::key_type& key) const {
	return object_base::at(key);
}

value_ptr object::operator[](const object_base::key_type& key) {
	return object_base::operator[](key);
}

value_ptr object::operator[](object_base::key_type&& key) {
	return object_base::operator[](std::move(key));
}

std::string object::str() const {
	std::ostringstream buf;
	buf << *this;
	return buf.str();
}

const char* object::parse(const char* json_text) {
	auto it = json_text;

	skip_space(it);

	// {
	if (*it == '{')
		skip_space(++it);
	else
		goto INVALID_FORMAT;

	if (*it == '}') // empty object
		return ++it;

	while (true) {
		// key(string)
		string key;
		if (*it == '"') {
			it = key.parse(it);
		}
		else {
			goto INVALID_FORMAT;
		}

		skip_space(it);

		// ':' separater
		if (*it == ':')
			skip_space(++it);
		else
			goto INVALID_FORMAT;

		// value
		std::shared_ptr<value> val;
		switch (*it) {
			case '{':
				val = std::make_shared<object>();
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

		// insert pair
		insert(std::make_pair(std::move(key), std::move(val)));

		skip_space(it);

		if (*it == ',')
			skip_space(++it);
		else if (*it == '}')
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

void object::parse(std::istream& is) {
	int ch;

	// {
	if ((ch = skip_space(is)) != '{')
		goto INVALID_FORMAT;

	if ((ch = skip_space(is)) == '}') // empty object
		return;

	while (true) {
		// key(string)
		string key;
		if (ch == '"') {
			is.unget();
			key.parse(is);
		}
		else {
			goto INVALID_FORMAT;
		}

		// ':' separater
		if ((ch = skip_space(is)) == ':')
			ch = skip_space(is);
		else
			goto INVALID_FORMAT;

		// value
		std::shared_ptr<value> val;
		switch (ch) {
			case '{':
				val = std::make_shared<object>();
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

		// insert pair
		insert(std::make_pair(std::move(key), std::move(val)));

		ch = skip_space(is);
		if (ch == ',')
			ch = skip_space(is);
		else if (ch == '}')
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

std::ostream& shan::json::operator<<(std::ostream& os, const object& json) {
	os << "{";

	for (auto it = json.cbegin() ; it != json.cend() ; ) {
		os << it->first << ":";

//		auto& ti = typeid(*(it->second.get())); // replaced by next 2 lines because of a warning. (Is this compiler bug?)
		auto temp = it->second.get();
		auto& ti = typeid(*temp);
		if (ti == typeid(object))
			os << *(std::static_pointer_cast<object>(it->second));
		else if (ti == typeid(array))
			os << *(std::static_pointer_cast<array>(it->second));
		else if (ti == typeid(string))
			os << *(std::static_pointer_cast<string>(it->second));
		else if (ti == typeid(number))
			os << *(std::static_pointer_cast<number>(it->second));
		else if (ti == typeid(true_value))
			os << *(std::static_pointer_cast<true_value>(it->second));
		else if (ti == typeid(false_value))
			os << *(std::static_pointer_cast<false_value>(it->second));
		else if (ti == typeid(null_value))
			os << *(std::static_pointer_cast<null_value>(it->second));

		it++;
		if (it == json.cend())
			break;

		os << ',';
	}

	return (os << "}");
}

std::istream& shan::json::operator>>(std::istream& is, object& json) {
	json.parse(is);
	return is;
}
