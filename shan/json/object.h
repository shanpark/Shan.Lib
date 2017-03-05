//
//  object.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_object_h
#define shan_json_object_h

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include "string.h"

namespace shan {
namespace json {

class object : public value, public std::map<string, std::shared_ptr<value>> {
public:
	object() {};
	object(const char* obj_str);
	object(const object& obj) = default;
	object(object&& obj) = default;

	virtual ~object() = default;

	object& operator=(const object& other) = default;
	object& operator=(object&& other) = default;

	virtual const char* parse(const char* json_text);

	virtual std::string str() const;

	friend std::ostream& operator<<(std::ostream& os, const object& json);
};

std::ostream& operator<<(std::ostream& os, const object& json);

} // namespace json
} // namespace shan

#endif /* shan_json_object_h */
