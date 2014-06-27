#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "ezjson.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <ctime>
#include <errno.h>
#include <vector>

int main()
{
	// detect memory leak
	{
		std::ifstream in("test5.txt", std::ios::in | std::ios::binary);
		if (in)
		{
			std::string contents;
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();

			clock_t t = clock();
			for (int i = 0; i < 500; ++i)
				EzJSON j = EzJSON(contents.c_str());
			//j.remove("gutter");
			//std::cout << j.serialize();
			//std::cout << j["nihao"].serialize();
			//std::cout << j["instruments"].serialize() << std::endl;
			std::cout << clock() - t << "\n";

		}
	}
	 _CrtDumpMemoryLeaks();
	return 0;
}