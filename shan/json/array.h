//
//  array.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_array_h
#define shan_json_array_h

namespace shan {
namespace json {

class object;

class array : public value, public array_base {
public:
	array() {};
	array(const char* arr_str) { array::parse(arr_str); }
	array(const std::string& arr_str) { array::parse(arr_str.c_str()); }
	array(std::istream& is) { array::parse(is); }

	std::size_t size() const noexcept { return array_base::size(); }

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

	virtual std::string str() const {
		std::ostringstream buf;
		buf << *this;
		return buf.str();
	}

	virtual const char* parse(const char* json_text);
	virtual void parse(std::istream& is);

	friend std::ostream& operator<<(std::ostream& os, const array& arr);
	friend std::istream& operator>>(std::istream& is, array& arr);
};

} // namespace json
} // namespace shan

#include "object.h"

namespace shan {
namespace json {

inline void array::add(const shan::json::object& obj) {
	push_back(std::make_shared<shan::json::object>(obj));
}

inline void array::add(shan::json::object&& obj) {
	push_back(std::make_shared<shan::json::object>(std::move(obj)));
}

inline const char* array::parse(const char* json_text) {
	auto it = json_text;

	__skip_sp(it);

	// [
	if (*it == '[')
		__skip_sp(++it);
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
				if ((*it == '-') || (std::isdigit(*it)))
					val = std::make_shared<number>();
				else
					goto INVALID_FORMAT;
				break;
		}
		it = val->parse(it);

		// push value
		push_back(std::move(val));

		__skip_sp(it);

		if (*it == ',')
			__skip_sp(++it);
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

inline void array::parse(std::istream& is) {
	int ch;

	// [
	if ((ch = __skip_sp(is)) != '[')
		goto INVALID_FORMAT;

	if ((ch = __skip_sp(is)) == ']') // empty array
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
				if ((ch == '-') || (std::isdigit(ch)))
					val = std::make_shared<number>();
				else
					goto INVALID_FORMAT;
				break;
		}
		is.unget();
		val->parse(is);

		// push value
		push_back(std::move(val));

		ch = __skip_sp(is);
		if (ch == ',')
			ch = __skip_sp(is);
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

inline std::ostream& operator<<(std::ostream& os, const array& arr) {
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

inline std::istream& operator>>(std::istream& is, array& arr) {
	arr.parse(is);
	return is;
}

} // namespace json
} // namespace shan

#endif /* shan_json_array_h */
