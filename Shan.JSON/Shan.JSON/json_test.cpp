//
//  json_test.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 3. 2..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <iostream>
#include <sstream>
#include "json_test.h"
#include "json/json.h"

using namespace shan::json;

void empty_json() {
	std::cout << "1. Empty JSON";

	try {
		std::cout << std::endl << "\t";

		shan::json::object j1(u8"{}");
		if (j1.empty())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::stringstream is(u8"{}");
		shan::json::object j2(is);
		if (j2.empty())
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

void general_json() {
	std::cout << "2. General JSON parsing";

	try {
		std::string res(u8R"({"arr":[1,2.0,true,false,null,[1,2.0,true,false],{"int":123}],"false":false,"int":12345,"null":null,"obj":{"int":123,"real":12.3},"real":123.45,"string":"12345","true":true})");
		std::string input(R"(
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

		std::cout << std::endl << "\t";

		object j1(input);
		if ((j1.size() == 8) && (j1.str() == res))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::stringstream is(input);
		object j2(is);
		if ((j2.size() == 8) && (j2.str() == res))
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

void array_test() {
	std::cout << "3. Array test";

	try {
		std::string input(u8"{\"array\":[1, 2.0, true, false, null, [1, 2.0, true, false], {\"int\":123}]}");
		object json(input);

		std::cout << std::endl << "\t";

		if (json["array"]->at(0)->int_val() == 1)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["array"]->at(1)->real_val() == 2.0)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["array"]->at(2)->bool_val())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (!json["array"]->at(3)->bool_val())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["array"]->at(4)->is_null())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";
		
		std::cout << std::endl << "\t"; //====

		if (json["array"]->at(5)->size() == 4)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["array"]->at(6)->size() == 1)
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

void number_test() {
	std::cout << "4. Number test";

	try {
		std::string input(u8R"(
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

		std::stringstream is(input);
		object json(is);

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

		std::cout << std::endl << "\t"; //====

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

		std::cout << std::endl << "\t"; //====

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
	std::cout << "5. String test";

	try {
		std::string input(u8R"(
		{
			"strs":["hello", "\b", "\f", "\n", "\r", "\t", "\"", "\\", "\/", "\uac00", "가", "\uD834\uDD1E", "𝄞"]
		}
						  )"); // [AC00]=가, [D834, DD1E]=𝄞

		std::istringstream is(input);
		shan::json::object json(is);
		std::cout << std::endl << "\t";

		if ((*json["strs"])[0]->str() == "hello")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(1)->str() == std::string("\b"))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(2)->str() == "\f")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(3)->str() == "\n")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(4)->str() == "\r")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t"; //====

		if (json["strs"]->at(5)->str() == "\t")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(6)->str() == "\"")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(7)->str() == "\\")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(8)->str() == "/")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(9)->str() == u8"가")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t"; //====

		if (json["strs"]->at(10)->str() == u8"가")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(10)->json_str() == u8"\"가\"")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(11)->str() == u8"𝄞")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json["strs"]->at(11)->json_str() == u8"\"𝄞\"")
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

void object_add_test() {
	std::cout << "6. Add to object test";

	try {
		std::cout << std::endl << "\t";

		shan::json::object json;
		json.add("true", true);
		if (json["true"]->bool_val())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		json.add(std::string("false"), false);
		if (!(json["false"]->bool_val()))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		json.add(shan::json::string("int_val"), 10);
		if (json["int_val"]->int_val() == 10)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		json.add("real_val", 3.14);
		if (json["real_val"]->real_val() == 3.14)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		json.add(shan::json::string("null"), nullptr);
		if (json["null"]->is_null())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t"; //=========

		json.add("const char*", "const char*");
		if (json["const char*"]->str() == "const char*")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::string s("std::string");
		json.add("std::string", s);
		if (json["std::string"]->str() == "std::string")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		json.add("std::string_move", std::move(s));
		if ((json["std::string_move"]->str() == "std::string") && (s.length() == 0))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		string s2("string");
		json.add("string", s2);
		if (json["string"]->str() == "string")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		json.add("string_move", std::move(s2));
		if ((json["string_move"]->str() == "string") && (s.length() == 0))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t"; //=========

		array arr("[1, 2]");
		json.add("array", arr);
		if (json["array"]->size() == 2)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		json.add("array_move", std::move(arr));
		if ((json["array_move"]->size() == 2) && arr.empty())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		object obj("{\"first\":1, \"second\":2}");
		json.add("object", obj);
		if (json["object"]->size() == 2)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		json.add("object_move", std::move(obj));
		if ((json["object_move"]->size() == 2) && obj.empty())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (json.size() == 14)
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

void array_add_test() {
	std::cout << "7. Add to array test";

	try {
		std::cout << std::endl << "\t";

		shan::json::array arr;
		arr.add(true);
		if (arr[0]->bool_val())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		arr.add(false);
		if (!(arr[1]->bool_val()))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		arr.add(10);
		if (arr[2]->int_val() == 10)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		arr.add(3.14);
		if (arr[3]->real_val() == 3.14)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		arr.add(nullptr);
		if (arr[4]->is_null())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t"; //=========

		arr.add("const char*");
		if (arr[5]->str() == "const char*")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::string s("std::string");
		arr.add(s);
		if (arr[6]->str() == "std::string")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		arr.add(std::move(s));
		if ((arr[7]->str() == "std::string") && (s.length() == 0))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		string s2("string");
		arr.add(s2);
		if (arr[8]->str() == "string")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		arr.add(std::move(s2));
		if ((arr[9]->str() == "string") && (s.length() == 0))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t"; //=========

		array a1("[1, 2]");
		arr.add(a1);
		if (arr[10]->size() == 2)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		arr.add(std::move(a1));
		if ((arr[11]->size() == 2) && a1.empty())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		object obj("{\"first\":1, \"second\":2}");
		arr.add(obj);
		if (arr[12]->size() == 2)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		arr.add(std::move(obj));
		if ((arr[13]->size() == 2) && obj.empty())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (arr.size() == 14)
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

void test_shan_json() {
	empty_json();
	general_json();
	array_test();
	number_test();
	string_test();
	object_add_test();
	array_add_test();
}
