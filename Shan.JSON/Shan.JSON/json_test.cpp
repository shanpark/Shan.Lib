//
//  json_test.cpp
//  Shan.JSON
//
//  Created by Sung Han Park on 2017. 3. 2..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//
// Note: use encoding UTF-8 with "BOM" in MS Visual Studio.
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

		if (j1.str() == "{}")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::stringstream is(u8"{}");
		shan::json::object j2(is);
		if (j2.empty())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (j2.json_str() == "{}")
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
		std::string input(u8R"(
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

		if (j1["int"]->int_val() == 12345)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (j1["real"]->real_val() == 123.45)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (j1["true"]->bool_val())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (!(j1["false"]->bool_val()))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t"; //====

		if (j1["null"]->is_null())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (j1["arr"]->at(0)->int_val() == 1)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (j1["obj"]->at("int")->int_val() == 123)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::stringstream is(input);
		object j2(is);
		if ((j2.size() == 8) && (j2.str() == res))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::stringstream is2(input);
		object j3;
		is2 >> j3; // >> operator test
		if ((j3.size() == 8) && (j3.str() == res))
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
		std::string input(u8"[1, 2.0, true, false, null, [1, 2.0, true, false], {\"int\":123}]");
		array arr(input);

		std::cout << std::endl << "\t";

		if (arr.at(0)->int_val() == 1)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (arr[1]->real_val() == 2.0)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (arr.at(2)->bool_val())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (!arr[3]->bool_val())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (arr[4]->is_null())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";
		
		std::cout << std::endl << "\t"; //====

		auto& ar = *arr[5];
		if (ar.size() == 4)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		auto& obj = *arr[6];
		if (obj.size() == 1)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::stringstream is(u8"[1, 2, 3, 4, 5, null]");
		shan::json::array arr2;
		is >> arr2; // >> operator test
		if (arr2.size() == 6)
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

		if ((*json["strs"])[0]->json_str() == "\"hello\"")
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

void pack_test() {
	std::cout << "8. MsgPack test";

	try {
		std::cout << std::endl << "\t";

		std::string input("{\"compact\":true, \"schema\":0}");
		shan::json::object js(input);

		std::vector<uint8_t> packed;
		js.pack(packed);
		if (packed.size() == 18)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (js.pack().size() == 18)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		shan::json::object ori(u8R"(
{
	"hangul":"가나다라마",
	"minus" : -31,
	"plus"  : 127,
	"bignum": 65536127,
	"double": 3.14,
	"true"  : true,
	"false" : false,
	"null"  : null,
	"arr" : [1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
			 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
			 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
			 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
			 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
			 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0],
	"obj" : {"0":0, "1":1, "2":3, "3":3, "4":4, "5":5, "6":6, "7":7, "8":8, "9":9, "10":0, "11":1, "12":3, "13":3, "14":4, "15":5, "16":6, "17":7, "18":8, "19":9,
		"20":0, "21":1, "22":3, "23":3, "24":4, "25":5, "26":6, "27":7, "28":8, "29":9, "30":0, "31":1, "32":3, "33":3, "34":4, "35":5, "36":6, "37":7, "38":8, "39":9,
		"40":0, "41":1, "42":3, "43":3, "44":4, "45":5, "46":6, "47":7, "48":8, "49":9, "50":0, "51":1, "52":3, "53":3, "54":4, "55":5, "56":6, "57":7, "58":8, "59":9,
		"60":0, "61":1, "62":3, "63":3, "64":4, "65":5, "66":6, "67":7, "68":8, "69":9, "70":0, "71":1, "72":3, "73":3, "74":4, "75":5, "76":6, "77":7, "78":8, "79":9,
		"80":0, "81":1, "82":3, "83":3, "84":4, "85":5, "86":6, "87":7, "88":8, "89":9, "90":0, "91":1, "92":3, "93":3, "94":4, "95":5, "96":6, "97":7, "98":8, "99":9},
	"longstr":"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
}
							   )");
		auto bytes(ori.pack());
		shan::json::object copy;
		copy.unpack(bytes);

		if (copy["hangul"]->str() == u8"가나다라마")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (copy["minus"]->int_val() == -31)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (copy["plus"]->int_val() == 127)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t"; //=========

		if (copy["bignum"]->int_val() == 65536127)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (copy["double"]->real_val() == 3.14)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (copy["true"]->bool_val())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (!(copy["false"]->bool_val()))
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (copy["null"]->is_null())
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t"; //=========

		if (copy["arr"]->size() == 300)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (copy["obj"]->size() == 100)
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (copy["longstr"]->str().length() == 300)
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
	pack_test();
}
