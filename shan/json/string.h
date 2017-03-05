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
	string(const char* str);
	string(const std::string& str);
	string(std::string&& str);
	string(const string& str) = default;
	string(string&& str) = default;

	virtual ~string() = default;

	string& operator=(const string& other) = default;
	string& operator=(string&& other) = default;

	virtual const char* parse(const char* json_text);

	virtual std::string str() const { return static_cast<std::string>(*this); }

	friend std::ostream& operator<<(std::ostream& os, const string& arr);
};

// operator ==
bool operator==(const string& lhs, const string& rhs) noexcept;
bool operator==(const char* lhs, const string& rhs) noexcept;
bool operator==(const string& lhs, const char* rhs) noexcept;

// operator <<
std::ostream& operator<<(std::ostream& os, const string& str);

} // namespace json
} // namespace shan

#endif /* shan_json_string_h */
