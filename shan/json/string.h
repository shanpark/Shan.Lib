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
	string(const char* str) : std::string(str) {}
	string(const std::string& str) : std::string(str) {}
	string(std::string&& str) : std::string(std::move(str)) {}

	virtual std::string str() const { return static_cast<std::string>(*this); }
	virtual std::string json_str() const; // in JSON format.

	virtual const char* parse(const char* json_text);
	virtual void parse(std::istream& is);

	friend std::ostream& operator<<(std::ostream& os, const string& str);
	friend std::istream& operator>>(std::istream& is, string& str);
};

// operator ==
bool operator==(const string& lhs, const string& rhs) noexcept;
bool operator==(const char* lhs, const string& rhs) noexcept;
bool operator==(const string& lhs, const char* rhs) noexcept;
bool operator==(const std::string& lhs, const string& rhs) noexcept;
bool operator==(const string& lhs, const std::string& rhs) noexcept;

// operator <<
std::ostream& operator<<(std::ostream& os, const string& str);
std::istream& operator>>(std::istream& is, string& str);

} // namespace json
} // namespace shan

#endif /* shan_json_string_h */
