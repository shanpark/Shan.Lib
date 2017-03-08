//
//  value.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 3. 3..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "value.h"
#include "exception.h"
#include "object.h"
#include "array.h"
#include "number.h"
#include "true_value.h"
#include "false_value.h"
#include "null_value.h"

using namespace shan::json;

std::size_t value::size() const {
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::size();
	else if (typeid(*this) == typeid(array))
		return as<shan::json::array>().array_base::size();
	else
		throw not_allowed_error("The object is not a JSON object or an array. [size]");
}

value_ptr value::at(const object_base::key_type& key) { // for object
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::at(key);
	else
		throw not_allowed_error("The object is not a JSON object. [at]");
}

const_value_ptr value::at(const object_base::key_type& key) const { // for object
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::at(key);
	else
		throw not_allowed_error("The object is not a JSON object. [at]");
}

value_ptr value::operator[](const object_base::key_type& key) { // for object
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::operator[](key);
	else
		throw not_allowed_error("The object is not a JSON object. [operator[]]");
}

value_ptr value::operator[](object_base::key_type&& key) { // for object
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::operator[](std::move(key));
	else
		throw not_allowed_error("The object is not a JSON object. [operator[]]");
}

value_ptr value::at(array_base::size_type pos) { // for array
	if (typeid(*this) == typeid(array))
		return as<array>().array_base::at(pos);
	else
		throw not_allowed_error("The object is not an array. [at]");
}

const_value_ptr value::at(array_base::size_type pos) const { // for array
	if (typeid(*this) == typeid(array))
		return as<array>()[pos];
	else
		throw not_allowed_error("The object is not an array. [at]");
}

value_ptr value::operator[](array_base::size_type pos) { // for array
	if (typeid(*this) == typeid(array))
		return as<array>().array_base::operator[](pos);
	else
		throw not_allowed_error("The object is not an array. [operator[]]");
}

const_value_ptr value::operator[](array_base::size_type pos) const { // for array
	if (typeid(*this) == typeid(array))
		return as<array>().array_base::operator[](pos);
	else
		throw not_allowed_error("The object is not an array. [operator[]]");
}

int64_t value::int_val() const {
	if (typeid(*this) == typeid(number))
		return as<number>().int_val();
	else
		throw not_allowed_error("The object can not be converted to integer. [int_val]");
}

double value::real_val() const {
	if (typeid(*this) == typeid(number))
		return as<number>().real_val();
	else
		throw not_allowed_error("The object can not be converted to double. [real_val]");
}

bool value::bool_val() const {
	if (typeid(*this) == typeid(true_value))
		return true;
	else if (typeid(*this) == typeid(false_value))
		return false;
	else
		throw not_allowed_error("The object can not be converted to bool. [bool_val]");
}

bool value::is_null() const {
	return typeid(*this) == typeid(null_value) ? true : false;
}
