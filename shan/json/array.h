//
//  array.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_array_h
#define shan_json_array_h

#include <iostream>
#include <memory>
#include <string>
#include "string.h"
#include "number.h"
#include "true_value.h"
#include "false_value.h"
#include "null_value.h"

namespace shan {
namespace json {

class object;

class array : public value, public array_base {
public:
	array() {};
	array(const char* arr_str) { array::parse(arr_str); }
	array(const std::string& arr_str) { array::parse(arr_str.c_str()); }
	array(std::istream& is) { array::parse(is); }

	std::size_t size() const { return array_base::size(); }

	// bool
	void add(bool bool_val) {
		if (bool_val)
			push_back(std::make_shared<true_value>());
		else
			push_back(std::make_shared<false_value>());
	}

	// null
	void add(std::nullptr_t) {
		push_back(std::make_shared<null_value>());
	}

	// string
	void add(const char* str) {
		push_back(std::make_shared<string>(str));
	}
	void add(const std::string& str) {
		push_back(std::make_shared<string>(str));
	}
	void add(std::string&& str) {
		push_back(std::make_shared<string>(std::move(str)));
	}
	void add(const string& str) {
		push_back(std::make_shared<string>(str));
	}
	void add(string&& str) {
		push_back(std::make_shared<string>(std::move(str)));
	}

	// number
	void add(const number& num) {
		push_back(std::make_shared<number>(num));
	}
	void add(number&& num) {
		push_back(std::make_shared<number>(std::move(num)));
	}
	template <typename NUM>
	void add(NUM num) {
		static_assert(std::is_integral<NUM>::value || std::is_floating_point<NUM>::value, "This method can only add a number type value.");
		push_back(std::make_shared<number>(num));
	}

	// array
	void add(const array& arr) {
		push_back(std::make_shared<array>(arr));
	}
	void add(array&& arr) {
		push_back(std::make_shared<array>(std::move(arr)));
	}

	// object
	void add(const shan::json::object& obj);
	void add(shan::json::object&& obj);

	value_ptr at(array_base::size_type pos) { return array_base::at(pos); }
	const_value_ptr at(array_base::size_type pos) const { return array_base::at(pos); }
	value_ptr operator [](array_base::size_type pos) { return array_base::operator[](pos); }
	const_value_ptr operator [](array_base::size_type pos) const { return array_base::operator[](pos); }

	virtual std::string str() const;

	virtual const char* parse(const char* json_text);
	virtual void parse(std::istream& is);

	friend std::ostream& operator<<(std::ostream& os, const array& arr);
	friend std::istream& operator>>(std::istream& is, array& arr);
};

std::ostream& operator<<(std::ostream& os, const array& arr);
std::istream& operator>>(std::istream& is, array& arr);

} // namespace json
} // namespace shan


#endif /* shan_json_array_h */
