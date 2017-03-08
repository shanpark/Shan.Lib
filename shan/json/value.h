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
#include <string>
#include <memory>
#include <map>
#include <vector>
#include "../object.h"

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

	int64_t int_val() const; // for number
	double real_val() const; // for number
	bool bool_val() const; // for true_value, false_value
	bool is_null() const; // for check null

	virtual std::string json_str() const { return str(); } // in JSON format.

	virtual const char* parse(const char* json_text) = 0;
	virtual void parse(std::istream& is) = 0;
};

inline void skip_space(const char*& pos) {
	while (std::isspace(*pos)) // skip spaces
		pos++;
}

inline int skip_space(std::istream& is) {
	int ch;
	while (std::isspace(ch = is.get())) // skip spaces
		;
	return ch;
}

} // namespace json
} // namespace shan

#endif /* shan_json_value_h */
