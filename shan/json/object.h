//
//  object.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_object_h
#define shan_json_object_h

#include <iostream>
#include <memory>
#include <string>
#include "array.h"
#include "string.h"
#include "number.h"
#include "true_value.h"
#include "false_value.h"
#include "null_value.h"

namespace shan {
namespace json {

class value;

class object : public value, public object_base {
public:
	object() {};
	object(const char* obj_str) { object::parse(obj_str); }
	object(const std::string& obj_str) { object::parse(obj_str.c_str()); }
	object(std::istream& is) { object::parse(is); }

	std::size_t size() const { return object_base::size(); }

	// bool
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, bool bool_val) {
		if (bool_val)
			return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<true_value>()));
		else
			return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<false_value>()));
	}

	// null
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, std::nullptr_t) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<null_value>()));
	}

	// string
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, const char* str) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<string>(str)));
	}
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, const std::string& str) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<string>(str)));
	}
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, std::string&& str) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<string>(std::move(str))));
	}
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, const string& str) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<string>(str)));
	}
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, string&& str) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<string>(std::move(str))));
	}

	// number
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, const number& num) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<number>(num)));
	}
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, number&& num) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<number>(std::move(num))));
	}
	template <typename KEY, typename NUM>
	std::pair<iterator,bool> add(KEY&& key, NUM num) {
		static_assert(std::is_integral<NUM>::value || std::is_floating_point<NUM>::value, "This method can only add a number type value.");
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<number>(num)));
	}

	// array
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, const array& arr) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<array>(arr)));
	}
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, array&& arr) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<array>(std::move(arr))));
	}
	
	// object
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, const object& obj) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<object>(obj)));
	}
	template <typename KEY>
	std::pair<iterator,bool> add(KEY&& key, object&& obj) {
		return insert(std::make_pair(string(std::forward<KEY>(key)), std::make_shared<object>(std::move(obj))));
	}

	value_ptr at(const object_base::key_type& key);
	const_value_ptr at(const object_base::key_type& key) const;
	value_ptr operator[](const object_base::key_type& key);
	value_ptr operator[](object_base::key_type&& key);

	virtual std::string str() const;

	virtual const char* parse(const char* json_text);
	virtual void parse(std::istream& is);

	friend std::ostream& operator<<(std::ostream& os, const object& json);
	friend std::istream& operator>>(std::istream& is, object& json);
};

std::ostream& operator<<(std::ostream& os, const object& json);
std::istream& operator>>(std::istream& is, object& json);

} // namespace json
} // namespace shan

#endif /* shan_json_object_h */
