//
//  string.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_string_h
#define shan_json_string_h

#include <iostream>
#include <string>
#include "value.h"

namespace shan {
namespace json {

class string : public value, public std::string {
public:
	string() {};
	string(const char* json_text);

	virtual const char* parse(const char* json_text);

	friend std::ostream& operator<<(std::ostream& os, const string& arr);
};

std::ostream& operator<<(std::ostream& os, const string& str);

}
}

#endif /* shan_json_string_h */
