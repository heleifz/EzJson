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
		FILE *f = fopen("test2.txt", "rb");
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *str = new char[fsize + 1];
		fread(str, fsize, 1, f);
		fclose(f);

		str[fsize] = 0;

		clock_t t = clock();
		//for (int i = 0; i < 100; ++i)
			JSON j(str);

		std::cout << clock() - t << "\n";
		rapidjson::Document d;
		t = clock();
		//for (int i = 0; i < 100; ++i)
		d.Parse<0>(str);
		std::cout << "rapid json : " << clock() - t << "\n";

		delete [] str;
	}
	_CrtDumpMemoryLeaks();
	return 0;
}