//
//  object.h
//  Shan
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_object_h
#define shan_object_h

#include <string>

namespace shan {

class object {
public:
	virtual ~object() = default;

	// string convert
	virtual std::string str() const { return typeid(*this).name(); }
	operator std::string() const { return str(); }; // std::string casting operator
};

} // namespace shan

#endif /* shan_object_h */
