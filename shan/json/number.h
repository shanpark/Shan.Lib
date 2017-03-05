//
//  number.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_number_h
#define shan_json_number_h

#include <cstdint>
#include <iostream>
#include "value.h"

namespace shan {
namespace json {

class number : public value {
public:
	number() {};
	number(const char* num_str);
	number(const number& num) = default;
	number(number&& num) = default;

	virtual ~number() = default;

	number& operator=(const number& other) = default;
	number& operator=(number&& other) = default;

	virtual const char* parse(const char* json_text);

	virtual operator int() { return (int)int_val(); };

	int64_t int_val();
	double real_val();

	virtual std::string str() const { return _str_rep; }

	friend std::ostream& operator<<(std::ostream& os, const number& num);

private:
	bool _integral;
	union {
		int64_t _int;
		double _real;
	} _val;
	std::string _str_rep;
};

std::ostream& operator<<(std::ostream& os, const number& num);

} // namespace json
} // namespace shan

#endif /* shan_json_number_h */
