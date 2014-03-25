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
	// detect memory leak
	{
		JSON j("test5.txt");

		auto db = j.field("instruments").
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
		ofs << another;

	}
	_CrtDumpMemoryLeaks();
	return 0;
}