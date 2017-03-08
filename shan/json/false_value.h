//
//  false_value.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_false_value_h
#define shan_json_false_value_h

#include <iostream>
#include "value.h"

namespace shan {
namespace json {

class false_value : public value {
public:
	false_value() {};

	virtual std::string str() const { return "false"; }

	virtual const char* parse(const char* json_text);
	virtual void parse(std::istream& is);

	friend std::ostream& operator<<(std::ostream& os, const false_value& f);
	friend std::istream& operator>>(std::istream& is, false_value& f);
};

std::ostream& operator<<(std::ostream& os, const false_value& f);
std::istream& operator>>(std::istream& is, false_value& f);

} // namespace json
} // namespace shan

#endif /* shan_json_false_value_h */
