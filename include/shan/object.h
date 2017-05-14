//
//  object.h
//  Shan
//
//  Created by Sung Han Park on 2017. 2. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_object_h
#define shan_object_h

#include <iostream>
#include <string>
#include <typeinfo>
#include <memory>

namespace shan {

class object {
public:
	virtual ~object() = default;

	virtual std::size_t hash() const {
		return reinterpret_cast<size_t>(const_cast<object*>(this));
	}

	// string convert
	virtual std::ostream& str(std::ostream& os) const { return os << typeid(*this).name(); }
	virtual std::string str() const { return typeid(*this).name(); }
	operator std::string() const { return str(); }; // std::string casting operator
};

using object_ptr = std::shared_ptr<object>;

inline std::ostream& operator<<(std::ostream& os, const object& o) {
	return o.str(os);
}

} // namespace shan

namespace std {

template <>
class hash<shan::object> {
public:
	size_t operator()(const shan::object& obj) const {
		return obj.hash();
	}
};

} // namespace std

#endif /* shan_object_h */
