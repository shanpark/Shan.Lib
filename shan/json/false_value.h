//
//  false_value.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_false_value_h
#define shan_json_false_value_h

namespace shan {
namespace json {

class false_value : public value {
public:
	false_value() {};

	virtual std::string str() const { return "false"; }

	virtual const char* parse(const char* json_text)  {
		auto it = json_text;

		if (*it == 'f')
			it++;
		else
			goto INVALID_FORMAT;

		if (*it == 'a')
			it++;
		else
			goto INVALID_FORMAT;

		if (*it == 'l')
			it++;
		else
			goto INVALID_FORMAT;

		if (*it == 's')
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

		if ((ch = is.get()) != 'f')
			goto INVALID_FORMAT;
		if ((ch = is.get()) != 'a')
			goto INVALID_FORMAT;
		if ((ch = is.get()) != 'l')
			goto INVALID_FORMAT;
		if ((ch = is.get()) != 's')
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

	friend std::ostream& operator<<(std::ostream& os, const false_value& f);
	friend std::istream& operator>>(std::istream& is, false_value& f);
};

inline std::ostream& operator<<(std::ostream& os, const false_value& f) {
	return (os << "false");
}

inline std::istream& operator>>(std::istream& is, false_value& f) {
	f.parse(is);
	return is;
}

} // namespace json
} // namespace shan

#endif /* shan_json_false_value_h */
