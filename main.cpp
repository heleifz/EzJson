#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "EzJson.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <ctime>


int main()
{
	//int position = 0;
	//std::ifstream is("test.txt");
	//auto in = std::istreambuf_iterator<char>(is);
	//std::string s(in, std::istreambuf_iterator<char>());;
	//const char* f = s.c_str();
	//Scanner scan(f);
	//while (1)
	//{
	//	if (scan.lookahead() == TokenType::EOS)
	//		break;
	//	std::cout << scan.lookahead() << " && " << scan.text() << std::endl;
	//	scan.next();
	//}

	// detect memory leak
	{
		clock_t t = clock();
		JSON j("test5.txt");
		std::cout << clock() - t;
		/*auto db = j.field("instruments").
			at(0).
			field("default_pan").
			asDouble();

			std::cout << db;

			JSON other = JSON::makeObject().
			set("hello", JSON::fromNumber(3)).
			set("world", JSON::fromBool(true)).
			set("hello", JSON::fromString("nihao")).
			set("f", JSON::fromString("F word"));

			JSON another = JSON::makeArray().append(other).append(JSON::fromNumber(4.3));

			std::cout << another;

			std::ofstream ofs("testout.txt");
			ofs << another;*/

	}
	_CrtDumpMemoryLeaks();
	return 0;
}