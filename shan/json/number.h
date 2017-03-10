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

	int64_t int_val() const { return _integral ? _val._int : static_cast<int64_t>(_val._real); }
	double real_val() const { return _integral ? static_cast<double>(_val._int) : _val._real; }

	virtual std::string str() const { return _str_rep; }

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
		if (static_cast<bool>(*it))
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
