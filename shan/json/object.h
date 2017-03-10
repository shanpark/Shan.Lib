//
//  object.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_object_h
#define shan_json_object_h

namespace shan {
namespace json {

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

	value_ptr at(const object_base::key_type& key) { return object_base::at(key); }
	const_value_ptr at(const object_base::key_type& key) const { return object_base::at(key); }
	value_ptr operator[](const object_base::key_type& key) { return object_base::operator[](key); }
	value_ptr operator[](object_base::key_type&& key) { return object_base::operator[](std::move(key)); }

	virtual std::string str() const {
		std::ostringstream buf;
		buf << *this;
		return buf.str();
	}

	virtual const char* parse(const char* json_text) {
		auto it = json_text;

		__skip_sp(it);

		// {
		if (*it == '{')
			__skip_sp(++it);
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

			__skip_sp(it);

			// ':' separater
			if (*it == ':')
				__skip_sp(++it);
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

			__skip_sp(it);

			if (*it == ',')
				__skip_sp(++it);
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

	virtual void parse(std::istream& is) {
		int ch;

		// {
		if ((ch = __skip_sp(is)) != '{')
			goto INVALID_FORMAT;

		if ((ch = __skip_sp(is)) == '}') // empty object
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
			if ((ch = __skip_sp(is)) == ':')
				ch = __skip_sp(is);
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

			ch = __skip_sp(is);
			if (ch == ',')
				ch = __skip_sp(is);
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

	friend std::ostream& operator<<(std::ostream& os, const object& json);
	friend std::istream& operator>>(std::istream& is, object& json);
};

inline std::ostream& operator<<(std::ostream& os, const object& json) {
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

inline std::istream& operator>>(std::istream& is, object& json) {
	json.parse(is);
	return is;
}

} // namespace json
} // namespace shan

#endif /* shan_json_object_h */
