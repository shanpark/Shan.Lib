//
//  value.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 3. 3..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#include "value.h"
#include "exception.h"
#include "number.h"
#include "true_value.h"
#include "false_value.h"
#include "null_value.h"

using namespace shan::json;

int64_t value::int_val() {
	if (typeid(*this) == typeid(number))
		return as<number>().int_val();
	else
		throw not_allowed_error("The object can not be converted to integer. [int_val]");
}

double value::real_val() {
	if (typeid(*this) == typeid(number))
		return as<number>().real_val();
	else
		throw not_allowed_error("The object can not be converted to double. [int_val]");
}

bool value::bool_val() {
	if (typeid(*this) == typeid(true_value))
		return true;
	else if (typeid(*this) == typeid(false_value))
		return false;
	else
		throw not_allowed_error("The object can not be converted to bool. [bool_val]");
}

bool value::is_null() {
	return typeid(*this) == typeid(null_value) ? true : false;
}
