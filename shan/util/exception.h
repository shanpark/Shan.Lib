//
//  exception.h
//  Shan.Util
//
//  Created by Sung Han Park on 2017. 3. 27..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_util_exception_h
#define shan_util_exception_h

#include <exception>

namespace shan {
namespace util {

class buffer_error : public std::runtime_error {
public:
	explicit buffer_error(const std::string& what) : std::runtime_error(what) {}
	explicit buffer_error(const char* what) : std::runtime_error(what) {}
};

} // namespace net
} // namespace shan

#endif /* shan_util_exception_h */
