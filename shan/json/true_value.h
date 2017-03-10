//
//  true_value.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_true_value_h
#define shan_json_true_value_h

namespace shan {
namespace json {

class true_value : public value {
public:
	true_value() {};

	virtual std::string str() const { return "true"; }

	virtual const char* parse(const char* json_text) {
		auto it = json_text;

		if (*it == 't')
			it++;
		else
			goto INVALID_FORMAT;

		if (*it == 'r')
			it++;
		else
			goto INVALID_FORMAT;

		if (*it == 'u')
			it++;
		else
			goto INVALID_FORMAT;

		if (*it == 'e')
			it++;
		else
			goto INVALID_FORMAT;

		return it;

	INVALID_FORMAT:
		if (static_cast<bool>(*it))
			throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
		else
			throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
	}

	virtual void parse(std::istream& is) {
		int ch;

		if ((ch = is.get()) != 't')
			goto INVALID_FORMAT;
		if ((ch = is.get()) != 'r')
			goto INVALID_FORMAT;
		if ((ch = is.get()) != 'u')
			goto INVALID_FORMAT;
		if ((ch = is.get()) != 'e')
			goto INVALID_FORMAT;
		return;

	INVALID_FORMAT:
		if (ch == std::istream::traits_type::eof())
			throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
		else
			throw bad_format_error(std::string("Invalid JSON format: '") + (char)ch + "'");
	}

	friend std::ostream& operator<<(std::ostream& os, const true_value& t);
	friend std::istream& operator>>(std::istream& is, true_value& t);
};

inline std::ostream& operator<<(std::ostream& os, const true_value& t) {
	return (os << "true");
}

inline std::istream& operator>>(std::istream& is, true_value& t) {
	t.parse(is);
	return is;
}

} // namespace json
} // namespace shan

#endif /* shan_json_true_value_h */
