//
//  exception.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_exception_h
#define shan_json_exception_h

#include <exception>

namespace shan {
namespace json {

class json_error : public std::runtime_error {
public:
	explicit json_error(const std::string& what) : std::runtime_error(what) {}
	explicit json_error(const char* what) : std::runtime_error(what) {}
};

class bad_format_error : public json_error {
public:
	explicit bad_format_error(const std::string& what) : json_error(what) {}
	explicit bad_format_error(const char* what) : json_error(what) {}
};

class not_allowed_error : public json_error {
public:
	explicit not_allowed_error(const std::string& what) : json_error(what) {}
	explicit not_allowed_error(const char* what) : json_error(what) {}
};

} // namespace json
} // namespace shan

#endif /* shan_json_exception_h */
