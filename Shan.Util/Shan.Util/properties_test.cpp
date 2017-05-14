//
//  properties_test.cpp
//  Shan.Util
//
//  Created by Sung Han Park on 2017. 4. 29..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <shan/util/properties.h>

using namespace shan;

void properties_test() {
	std::cout << "1. Properties Test";
	util::properties prop;

	try {
		std::string input(u8R"(
	  color00:red
	  color01 = yellow
	  color02 = yellow red
	  color03 blue
	  color04
	  color05 \
		cyan
	  color06\
		=purple
	  color07 = gree\
	  n
	  color\
	  08 : whi\
	  te
	  color09        \

	  color10  \u0030

	  = none
	  # Comments
	  ! Comments
	  color11 = \
	  black
	  color12 = \uac00
	  color13 = \ud834\udd1e
						  )");

		std::istringstream is(input);
		prop.load(is);

		std::cout << std::endl << "\t";

		if (prop.get("color00") == "red")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color01") == "yellow")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color02") == "yellow red")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color03") == "blue")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color04") == "")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t";

		if (prop.get("color05") == "cyan")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color06") == "purple")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color07") == "green")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color08") == "white")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color09") == "")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t";

		if (prop.get("color10") == "0")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color11") == "black")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color12") == u8"가")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		if (prop.get("color13") == u8"𝄞")
			std::cout << "[OK]";
		else
			std::cout << "[Fail]";

		std::cout << std::endl << "\t";
	} catch (const shan::util::not_found_error& e) {
		std::cout << "Error: " << e.what() << std::endl;
	}
}
