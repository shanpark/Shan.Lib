//
//  null_value.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_null_value_h
#define shan_json_null_value_h

#include <iostream>
#include "value.h"

namespace shan {
namespace json {

class null_value : public value {
public:
	null_value() {};
	
	virtual const char* parse(const char* json_text);

	virtual std::string str() const { return "null"; }

	friend std::ostream& operator<<(std::ostream& os, const null_value& n);
};

std::ostream& operator<<(std::ostream& os, const null_value& n);

} // namespace json
} // namespace shan

#endif /* shan_json_null_value_h */
