//
//  array.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_array_h
#define shan_json_array_h

#include <iostream>
#include <memory>
#include <vector>
#include "value.h"

namespace shan {
namespace json {

class array : public value, public std::vector<std::shared_ptr<value>> {
public:
	array() {};
	array(const char* arr_str);
	array(const array& arr) = default;
	array(array&& arr) = default;

	virtual ~array() = default;

	array& operator=(const array& other) = default;
	array& operator=(array&& other) = default;

	virtual const char* parse(const char* json_text);

	virtual std::string str() const;

	friend std::ostream& operator<<(std::ostream& os, const array& arr);
};

std::ostream& operator<<(std::ostream& os, const array& arr);

} // namespace json
} // namespace shan


#endif /* shan_json_array_h */
