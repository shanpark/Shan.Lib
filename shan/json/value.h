//
//  value.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_json_value_h
#define shan_json_value_h

#include <sstream>
#include <typeinfo>
#include "../object.h"

namespace shan {
namespace json {

class value : public shan::object {
public:
	virtual const char* parse(const char* json_text) = 0;

	template<typename T>
	T& as() { return *(static_cast<T*>(this)); }

	int64_t int_val();
	double real_val();
	bool bool_val();
	bool is_null();
};

inline void skip_space(const char*& pos) {
	while (std::isspace(*pos)) // skip spaces
		pos++;
}

} // namespace json
} // namespace shan

#endif /* shan_json_value_h */
