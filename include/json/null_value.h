//
//  null_value.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_null_value_h
#define shan_json_null_value_h

namespace shan {
namespace json {

class null_value : public value {
public:
	null_value() {};

	virtual bool is_null() const { return true; }

	virtual std::string str() const { return "null"; }

	using value::pack;
	virtual std::vector<uint8_t>& pack(std::vector<uint8_t>& packed) const {
		packed.push_back(static_cast<uint8_t>(0xc0));
		return packed;
	};
	virtual util::streambuf& pack(util::streambuf& packed) const {
		packed.write_int8(static_cast<uint8_t>(0xc0));
		return packed;
	}

	using value::unpack;
	virtual const uint8_t* unpack(const uint8_t* bytes) {
		auto it = bytes;

		if (*it == 0xc0)
			it++;
		else
			throw bad_format_error(std::string("Invalid MessagePack format."));

		return it;
	}

	virtual const char* parse(const char* json_text) {
		auto it = json_text;

		if (*it == 'n')
			it++;
		else
			goto INVALID_FORMAT;

		if (*it == 'u')
			it++;
		else
			goto INVALID_FORMAT;

		if (*it == 'l')
			it++;
		else
			goto INVALID_FORMAT;

		if (*it == 'l')
			it++;
		else
			goto INVALID_FORMAT;

		return it;

	INVALID_FORMAT:
		if (*it != '\0')
			throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
		else
			throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
	}

	virtual void parse(std::istream& is) {
		int ch;

		if ((ch = is.get()) != 'n')
			goto INVALID_FORMAT;
		if ((ch = is.get()) != 'u')
			goto INVALID_FORMAT;
		if ((ch = is.get()) != 'l')
			goto INVALID_FORMAT;
		if ((ch = is.get()) != 'l')
			goto INVALID_FORMAT;
		return;

	INVALID_FORMAT:
		if (ch == std::istream::traits_type::eof())
			throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
		else
			throw bad_format_error(std::string("Invalid JSON format: '") + (char)ch + "'");
	}

	friend std::ostream& operator<<(std::ostream& os, const null_value& n);
	friend std::istream& operator>>(std::istream& is, null_value& n);
};

inline std::ostream& operator<<(std::ostream& os, const null_value& n) {
	return (os << "null");
}

inline std::istream& operator>>(std::istream& is, null_value& n) {
	n.parse(is);
	return is;
}

} // namespace json
} // namespace shan

#endif /* shan_json_null_value_h */
