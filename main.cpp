#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
// #include <crtdbg.h>

#include "EzJson.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <ctime>
#include <errno.h>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

int main()
{
	// detect memory leak
	{
		std::ifstream in("test6.txt", std::ios::in | std::ios::binary);
		if (in)
		{
			std::string contents;
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();

			clock_t t = clock();
			for (int i = 0; i < 1; ++i)
			JSON j(contents.c_str());
			std::cout << clock() - t << "\n";

			rapidjson::Document d;
			t = clock();
			for (int i = 0; i < 1; ++i)
				d.Parse<0>(contents.c_str());
			std::cout << "rapid json : " << clock() - t << "\n";

		}
	}
	// _CrtDumpMemoryLeaks();
	return 0;
}