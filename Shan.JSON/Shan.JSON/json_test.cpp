//
//  json_test.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 3. 2..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#include <iostream>
#include "json_test.h"
#include "json/json.h"

using namespace shan::json;

void empty_json() {
	std::cout << "1. Empty JSON" << std::endl;

	try {
		shan::json::object json(u8"{}");
		if (json.empty())
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;
	}
	catch (const std::exception &e) {
		std::cout << "\t[Fail:" << e.what() << "]" << std::endl;
	}
	catch (...) {
		std::cout << "\t[Fail:Unknown exception]" << std::endl;
	}
}

void general_json() {
	std::cout << "2. General JSON parsing" << std::endl;

	try {
		std::string res(u8R"({"arr":[1,2.0,true,false,null,[1,2.0,true,false],{"int":123}],"false":false,"int":12345,"null":null,"obj":{"int":123,"real":12.3},"real":123.45,"string":"12345","true":true})");
		object json(R"(
{
	 "int"   :  12345,
	 "real"  : 123.45,
	 "true"  :   true,
	 "false" :  false,
	 "null"  :   null,
	 "string":"12345",
	 "arr"   :[1, 2.0, true, false, null, [1, 2.0, true, false], {"int":123}],
	 "obj"   :{"int":123, "real":12.3}
}
					)");

		if ((json.size() == 8) && (json.str() == res))
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;
	}
	catch (const std::exception &e) {
		std::cout << "\t[Fail:" << e.what() << "]" << std::endl;
	}
	catch (...) {
		std::cout << "\t[Fail:Unknown exception]" << std::endl;
	}
}

void array_test() {
	std::cout << "3. Array test" << std::endl;

	try {
		object json(u8"{\"array\":[1, 2.0, true, false, null, [1, 2.0, true, false], {\"int\":123}]}");

		if (json.size() == 1) {
			if (json["array"]->as<array>().size() == 7)
				std::cout << "\t[OK]" << std::endl;
			else
				std::cout << "\t[Fail]" << std::endl;
		}
		else
			std::cout << "\t[Fail]" << std::endl;
	}
	catch (const std::exception &e) {
		std::cout << "\t[Fail:" << e.what() << "]" << std::endl;
	}
	catch (...) {
		std::cout << "\t[Fail:Unknown exception]" << std::endl;
	}
}

void number_test() {
	std::cout << "4. Number test";

	try {
		object json(u8R"(
{
	"zero1":0,
	"zero2":-0,
	"one":1,
	"int1":1234567890,
	"int2":2e1,
	"real1":2.5,
	"real2":-2.5,
	"real3":-0.0,
	"real4":2e1,
	"real5":-2.5E2,
	"real6":21.52e-12
}
					)");

		std::cout << std::endl << "\t";

		if (json["zero1"]->int_val() == 0)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["zero2"]->int_val() == 0)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["one"]->int_val() == 1)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["int1"]->int_val() == 1234567890)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["int2"]->int_val() == 2e1)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t";

		if (json["real1"]->real_val() == 2.5)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["real2"]->real_val() == -2.5)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["real3"]->real_val() == 0.0)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["real4"]->real_val() == 2e1)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["real5"]->real_val() == -2.5E2)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t";
		
		if (json["real6"]->real_val() == 21.52e-12)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl;
	}
	catch (const std::exception &e) {
		std::cout << "\t[Fail:" << e.what() << "]" << std::endl;
	}
	catch (...) {
		std::cout << "\t[Fail:Unknown exception]" << std::endl;
	}
}

void string_test() {
	std::cout << "5. String test" << std::endl;

	try {
		shan::json::object json(u8R"(
{
	"strs":["hello", "\b", "\f", "\n", "\r", "\t", "\"", "\\", "\/", "\uac00", "가", "\uD834\uDD1E"]
}
								)"); // [AC00]=가, [D834, DD1E]=𝄞

		std::cout << json << std::endl;
		
		if (json["strs"]->as<array>()[0]->str() == "hello")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[1]->as<string>() == "\b")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[2]->as<string>() == "\f")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[3]->as<string>() == "\n")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[4]->as<string>() == "\r")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[5]->as<string>() == "\t")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[6]->as<string>() == "\"")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[7]->as<string>() == "\\")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[8]->as<string>() == "/")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[9]->as<string>() == u8"가")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[10]->as<string>() == u8"가")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;

		if (json["strs"]->as<array>()[11]->as<string>() == u8"𝄞")
			std::cout << "\t[OK]" << std::endl;
		else
			std::cout << "\t[Fail]" << std::endl;
	}
	catch (const std::exception &e) {
		std::cout << "\t[Fail:" << e.what() << "]" << std::endl;
	}
	catch (...) {
		std::cout << "\t[Fail:Unknown exception]" << std::endl;
	}
}

void test_shan_json() {
	empty_json();
	general_json();
	array_test();
	number_test();
	string_test();
}
