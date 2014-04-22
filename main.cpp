#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "EzJson.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <ctime>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

int main()
{
	// detect memory leak
	{
		FILE *f = fopen("test6.txt", "rb");
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *str = new char[fsize + 1];
		fread(str, fsize, 1, f);
		fclose(f);

		str[fsize] = 0;

		clock_t t = clock();
		JSON j(str);
		std::cout << clock() - t << "\n";

		rapidjson::Document d;
		t = clock();
		d.Parse<0>(str);
		std::cout << "rapid json : " << clock() - t << "\n";

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