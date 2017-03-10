//
//  value.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_value_h
#define shan_json_value_h

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cctype>
#include <utility>
#include "../object.h"
#include "exception.h"
#include "unicode/utf.h"

namespace shan {
namespace json {

class string;
class value;

using value_ptr = std::shared_ptr<value>;
using const_value_ptr = std::shared_ptr<const value>;
using object_base = std::map<string, value_ptr>;
using array_base = std::vector<value_ptr>;

class value : public shan::object {
public:
	template<typename T>
	T& as() { return *(static_cast<T*>(this)); }
	template<typename T>
	const T& as() const { return *(static_cast<const T*>(this)); }

	std::size_t size() const; // for object, array

	// query real values
	value_ptr at(const object_base::key_type& key);             // for object
	const_value_ptr at(const object_base::key_type& key) const; // for object
	value_ptr operator[](const object_base::key_type& key);     // for object
	value_ptr operator[](object_base::key_type&& key);          // for object

	value_ptr at(array_base::size_type pos);                     // for array
	const_value_ptr at(array_base::size_type pos) const;         // for array
	value_ptr operator[](array_base::size_type pos);             // for array
	const_value_ptr operator[](array_base::size_type pos) const; // for array

	int64_t int_val() const;  // for number
	double real_val() const; // for number
	bool bool_val() const; // for true_value, false_value
	bool is_null() const; // for check null

	virtual std::string json_str() const { return str(); } // in JSON format.

	virtual const char* parse(const char* json_text) = 0;
	virtual void parse(std::istream& is) = 0;

protected:
	static void __skip_sp(const char*& pos) { // for parse()
		while (std::isspace(*pos)) // skip spaces
			pos++;
	}

	static int __skip_sp(std::istream& is) { // for parse()
		int ch;
		while (std::isspace(ch = is.get())) // skip spaces
			;
		return ch;
	}
};

} // namespace json
} // namespace shan

#include "true_value.h"
#include "false_value.h"
#include "null_value.h"
#include "number.h"
#include "string.h"
#include "array.h"

namespace shan {
namespace json {

inline std::size_t value::size() const {
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::size();
	else if (typeid(*this) == typeid(array))
		return as<shan::json::array>().array_base::size();
	else
		throw not_allowed_error("The object is not a JSON object or an array. [size]");
}

inline value_ptr value::at(const object_base::key_type& key) { // for object
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::at(key);
	else
		throw not_allowed_error("The object is not a JSON object. [at]");
}

inline const_value_ptr value::at(const object_base::key_type& key) const { // for object
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::at(key);
	else
		throw not_allowed_error("The object is not a JSON object. [at]");
}

inline value_ptr value::operator[](const object_base::key_type& key) { // for object
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::operator[](key);
	else
		throw not_allowed_error("The object is not a JSON object. [operator[]]");
}

inline value_ptr value::operator[](object_base::key_type&& key) { // for object
	if (typeid(*this) == typeid(shan::json::object))
		return as<shan::json::object>().object_base::operator[](std::move(key));
	else
		throw not_allowed_error("The object is not a JSON object. [operator[]]");
}

inline value_ptr value::at(array_base::size_type pos) { // for array
	if (typeid(*this) == typeid(array))
		return as<array>().array_base::at(pos);
	else
		throw not_allowed_error("The object is not an array. [at]");
}

inline const_value_ptr value::at(array_base::size_type pos) const { // for array
	if (typeid(*this) == typeid(array))
		return as<array>()[pos];
	else
		throw not_allowed_error("The object is not an array. [at]");
}

inline value_ptr value::operator[](array_base::size_type pos) { // for array
	if (typeid(*this) == typeid(array))
		return as<array>().array_base::operator[](pos);
	else
		throw not_allowed_error("The object is not an array. [operator[]]");
}

inline const_value_ptr value::operator[](array_base::size_type pos) const { // for array
	if (typeid(*this) == typeid(array))
		return as<array>().array_base::operator[](pos);
	else
		throw not_allowed_error("The object is not an array. [operator[]]");
}

inline int64_t value::int_val() const {
	if (typeid(*this) == typeid(number))
		return as<number>().int_val();
	else
		throw not_allowed_error("The object can not be converted to integer. [int_val]");
}

inline double value::real_val() const {
	if (typeid(*this) == typeid(number))
		return as<number>().real_val();
	else
		throw not_allowed_error("The object can not be converted to double. [real_val]");
}

inline bool value::bool_val() const {
	if (typeid(*this) == typeid(true_value))
		return true;
	else if (typeid(*this) == typeid(false_value))
		return false;
	else
		throw not_allowed_error("The object can not be converted to bool. [bool_val]");
}
	
inline bool value::is_null() const {
	return typeid(*this) == typeid(null_value) ? true : false;
}

} // namespace json
} // namespace shan

#endif /* shan_json_value_h */
