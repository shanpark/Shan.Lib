//
//  number.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_number_h
#define shan_json_number_h

namespace shan {
namespace json {

class number : public value {
public:
	number() : _integral(true) {}
	number(const char* num_str) { number::parse(num_str); }
	number(const std::string& num_str) { number::parse(num_str.c_str()); }
	template<typename T>
	number(T val) {
		static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "Parameter 'val' should be a number type.");
		_integral = std::is_integral<T>();
		if (_integral) {
			_val._int = static_cast<int64_t>(val);
			_str_rep = std::to_string(_val._int);
		}
		else {
			_val._real = static_cast<double>(val);
			_str_rep = double_to_string(_val._real);
		}
	}

	virtual bool is_number() const { return true; }

	int64_t int_val() const { return _integral ? _val._int : static_cast<int64_t>(_val._real); }
	double real_val() const { return _integral ? static_cast<double>(_val._int) : _val._real; }

	virtual std::string str() const { return _str_rep; }

	using value::pack;
	virtual std::vector<uint8_t>& pack(std::vector<uint8_t>& packed) const {
		if (_integral) {
			if ((_val._int >= -32ll) && (_val._int <= 127ll)) {
				packed.push_back(static_cast<uint8_t>(_val._int));
			}
			else if ((_val._int >= -128ll) && (_val._int <= 127ll)) {
				packed.push_back(static_cast<uint8_t>(0xd0));
				packed.push_back(static_cast<uint8_t>(_val._int & 0xff));
			}
			else if ((_val._int >= -32768ll) && (_val._int <= 32767ll)) {
				packed.push_back(static_cast<uint8_t>(0xd1));
				packed.push_back(static_cast<uint8_t>((_val._int >> 8) & 0xff));
				packed.push_back(static_cast<uint8_t>(_val._int & 0xff));
			}
			else if ((_val._int >= -2147483648ll) && (_val._int <= 2147483647ll)) {
				packed.push_back(static_cast<uint8_t>(0xd2));
				packed.push_back(static_cast<uint8_t>((_val._int >> 24) & 0xff));
				packed.push_back(static_cast<uint8_t>((_val._int >> 16) & 0xff));
				packed.push_back(static_cast<uint8_t>((_val._int >> 8) & 0xff));
				packed.push_back(static_cast<uint8_t>(_val._int & 0xff));
			}
			else { // if (_val._int >= -9223372036854775808) && (_val._int <= 9223372035854775807) {
				packed.push_back(static_cast<uint8_t>(0xd3));
				packed.push_back(static_cast<uint8_t>((_val._int >> 56) & 0xff));
				packed.push_back(static_cast<uint8_t>((_val._int >> 48) & 0xff));
				packed.push_back(static_cast<uint8_t>((_val._int >> 40) & 0xff));
				packed.push_back(static_cast<uint8_t>((_val._int >> 32) & 0xff));
				packed.push_back(static_cast<uint8_t>((_val._int >> 24) & 0xff));
				packed.push_back(static_cast<uint8_t>((_val._int >> 16) & 0xff));
				packed.push_back(static_cast<uint8_t>((_val._int >> 8) & 0xff));
				packed.push_back(static_cast<uint8_t>(_val._int & 0xff));
			}
		}
		else { // double
			packed.push_back(static_cast<uint8_t>(0xcb));
			uint64_t temp = util::hton64(_val._int);
			packed.push_back(static_cast<uint8_t>((temp & 0xff00000000000000) >> 56)); // _val._int == _val._
			packed.push_back(static_cast<uint8_t>((temp & 0xff000000000000) >> 48));
			packed.push_back(static_cast<uint8_t>((temp & 0xff0000000000) >> 40));
			packed.push_back(static_cast<uint8_t>((temp & 0xff00000000) >> 32));
			packed.push_back(static_cast<uint8_t>((temp & 0xff000000) >> 24));
			packed.push_back(static_cast<uint8_t>((temp & 0xff0000) >> 16));
			packed.push_back(static_cast<uint8_t>((temp & 0xff00) >> 8));
			packed.push_back(static_cast<uint8_t>(temp & 0xff));
		}

		return packed;
	};
	using value::unpack;
	virtual const uint8_t* unpack(const uint8_t* bytes) {
		auto it = bytes;

		if ((*it <= 0x7f) || (*it >= 0xe0)) {
			_integral = true;
			int8_t temp = *it;
			_val._int = temp;
		}
		else if (*it == 0xd0) {
			_integral = true;
			int8_t temp = *(++it);
			_val._int = temp;
		}
		else if (*it == 0xd1) {
			_integral = true;
			int16_t temp = *(++it);
			temp = (temp << 8) | *(++it);
			_val._int = temp;
		}
		else if (*it == 0xd2) {
			_integral = true;
			int32_t temp = *(++it);
			temp = (temp << 8) | *(++it);
			temp = (temp << 8) | *(++it);
			temp = (temp << 8) | *(++it);
			_val._int = temp;
		}
		else if (*it == 0xd3) {
			_integral = true;
			int64_t temp = *(++it);
			temp = (temp << 8) | *(++it);
			temp = (temp << 8) | *(++it);
			temp = (temp << 8) | *(++it);
			temp = (temp << 8) | *(++it);
			temp = (temp << 8) | *(++it);
			temp = (temp << 8) | *(++it);
			temp = (temp << 8) | *(++it);
			_val._int = temp;
		}
		else if (*it == 0xca) {
			_integral = false;
			union {
				int32_t _int;
				float _real;
			} temp;
			temp._int = *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = util::ntoh32(temp._int);
			_val._real = temp._real;
		}
		else if (*it == 0xcb) {
			_integral = false;
			union {
				int64_t _int;
				double _real;
			} temp;
			temp._int = *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = (temp._int << 8) | *(++it);
			temp._int = util::ntoh64(temp._int);
			_val._real = temp._real;
		}
		else
			throw bad_format_error(std::string("Invalid MessagePack format."));

		if (_integral)
			_str_rep = std::to_string(_val._int);
		else
			_str_rep = double_to_string(_val._real);

		return ++it;
	}

	virtual const char* parse(const char* json_text) {
		auto it = json_text;

		bool integral = true;
		std::string num;

		__skip_sp(it);

		if (*it == '-') {
			num.push_back(*it);
			it++;
		}

		if (isdigit(*it)) {
			num.push_back(*it);
			it++;
		}
		else {
			goto INVALID_FORMAT;
		}

		while (isdigit(*it)) {
			num.push_back(*it);
			it++;
		}

		if (*it == '.') {
			num.push_back(*it);
			it++;
			integral = false;

			if (isdigit(*it)) {
				num.push_back(*it);
				it++;
			}
			else {
				goto INVALID_FORMAT;
			}

			while (isdigit(*it)) {
				num.push_back(*it);
				it++;
			}
		}

		if ((*it == 'e') || (*it == 'E')) {
			num.push_back('e');
			it++;
			integral = false;

			if ((*it == '-') || (*it == '+')) {
				num.push_back(*it);
				it++;
			}

			if (isdigit(*it)) {
				num.push_back(*it);
				it++;
			}
			else {
				goto INVALID_FORMAT;
			}

			while (isdigit(*it)) {
				num.push_back(*it);
				it++;
			}
		}

		_integral = integral;
		_str_rep = num;
		if (_integral)
			_val._int = strtoll(_str_rep.c_str(), nullptr, 10);
		else
			_val._real = strtod(_str_rep.c_str(), nullptr);

		return it;

	INVALID_FORMAT:
		if (*it != '\0')
			throw bad_format_error(std::string("Invalid JSON format: '") + *it + "'");
		else
			throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
	}

	virtual void parse(std::istream& is) {
		bool integral = true;
		std::string num;
		int ch;

		if ((ch = __skip_sp(is)) == '-') {
			num.push_back(ch);
			ch = is.get();
		}

		if (isdigit(ch))
			num.push_back(ch);
		else
			goto INVALID_FORMAT;

		while (isdigit(ch = is.get()))
			num.push_back(ch);

		if (ch == '.') {
			num.push_back(ch);
			integral = false;

			if (isdigit(ch = is.get()))
				num.push_back(ch);
			else
				goto INVALID_FORMAT;

			while (isdigit(ch = is.get()))
				num.push_back(ch);
		}

		if ((ch == 'e') || (ch == 'E')) {
			num.push_back('e');
			integral = false;

			ch = is.get();
			if ((ch == '-') || (ch == '+')) {
				num.push_back(ch);
				ch = is.get();
			}

			if (isdigit(ch))
				num.push_back(ch);
			else
				goto INVALID_FORMAT;

			while (isdigit(ch = is.get()))
				num.push_back(ch);
		}

		is.unget(); // last ch should be returned to stream

		_integral = integral;
		_str_rep = num;
		if (_integral)
			_val._int = strtoll(_str_rep.c_str(), nullptr, 10);
		else
			_val._real = strtod(_str_rep.c_str(), nullptr);
		return;

	INVALID_FORMAT:
		if (ch == std::istream::traits_type::eof())
			throw bad_format_error(std::string("Invalid JSON format: unexpected end."));
		else
			throw bad_format_error(std::string("Invalid JSON format: '") + (char)ch + "'");
	}


	friend std::ostream& operator<<(std::ostream& os, const number& num);
	friend std::istream& operator>>(std::istream& is, number& num);

protected:
	std::string double_to_string(double val) {
		char buf[64];
		std::sprintf(buf, "%.15f", val);
		std::string str(buf);
		auto pos = str.find_last_not_of('0');
		return ((str[pos] == '.') && (pos < str.length() - 1)) ? str.erase(pos + 2) : str.erase(pos + 1);
	}

protected:
	bool _integral;
	union val {
		val() : _int(0) {}
		val(int64_t val) : _int(val) {}
		val(double val) : _real(val) {}

		int64_t _int;
		double _real;
	} _val;
	std::string _str_rep;
};

inline std::ostream& operator<<(std::ostream& os, const number& num) {
	return (os << num._str_rep);
}

inline std::istream& operator>>(std::istream& is, number& num) {
	num.parse(is);
	return is;
}

} // namespace json
} // namespace shan

#endif /* shan_json_number_h */
