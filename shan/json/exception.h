//
//  json_exception.h
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 2. 28..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#ifndef shan_json_exception_h
#define shan_json_exception_h

#include <exception>

namespace shan {
namespace json {

class bad_format_error : public runtime_error {
public:
	explicit bad_format_error(const string& what) : runtime_error(what) {}
	explicit bad_format_error(const char* what) : runtime_error(what) {}
};

}
}

#endif /* shan_json_exception_h */
