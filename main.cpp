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
		// read entire file into string
		std::ifstream is("test5.txt");
		std::ofstream os("testout.txt");
		if (is)
		{
			auto in = std::istreambuf_iterator<char>(is);
			std::string s(in, std::istreambuf_iterator<char>());

			std::cout << "file string size : " << s.size() << std::endl;
			clock_t t1 = clock();
			std::shared_ptr<Node> result;
			for (int i = 0; i != 1; ++i)
				result = Parser::parse(s);
			std::cout << "parse time : " << clock() - t1 << " ms\n";
			std::shared_ptr<Visitor> v = std::make_shared<PrintVisitor<std::ostream>>(os);
			// operation
			//std::cout << "chaining test:\n";
			//result->field("hahaha")->field("a")->acceptVisitor(v);
			//std::cout << "\n";

			std::cout << "file" << std::endl;
			std::cout << "=============== visitor test ======================\n";
			result->acceptVisitor(v);
		}
		else
		{
			std::cout << "Could not open test.txt\n";
		}
	}
	_CrtDumpMemoryLeaks();
	return 0;
}