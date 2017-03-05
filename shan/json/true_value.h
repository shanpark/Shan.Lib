//
//  true_value.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_true_value_h
#define shan_json_true_value_h

#include <iostream>
#include "value.h"

namespace shan {
namespace json {

class true_value : public value {
public:
	true_value() {};
	
	virtual const char* parse(const char* json_text);

	virtual std::string str() const { return "true"; }

	friend std::ostream& operator<<(std::ostream& os, const true_value& t);
};

std::ostream& operator<<(std::ostream& os, const true_value& t);

} // namespace json
} // namespace shan

#endif /* shan_json_true_value_h */
