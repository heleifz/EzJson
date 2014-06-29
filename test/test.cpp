#include "../ezjson/ezjson.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <assert.h>

std::string getFileContent(const std::string& path)
{
	std::ifstream file(path);
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

void testSpeed(const std::string& filepath, int N = 100)
{
	clock_t clk;
	// Test multiple json file
	auto f1 = getFileContent(filepath);
	std::cout << "Test speed for file " << filepath << " (" << (f1.size() / 1024.0) << " KB) ... \n";
	clk = clock();
	for (int i = 0; i < N; ++i)
	{
		Ez::JSON j(f1.c_str());
	}
	std::cout << ">>> " << ((clock() - clk) / double(N)) << " ms\n";
}

void testErrorHandling(const char *json)
{
	std::cout << "Input String : " << json << "\n";
	std::cout << ">> Fail with error : ";
	try
	{
		Ez::JSON j(json);
		std::cout << "NONE\n";
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << "\n";
	}
}

void testPrettyPrint(const char *json)
{
	std::cout << "Input String : " << json << "\n";
	std::cout << ">> Pretty print : \n";
	Ez::JSON j(json);
	std::cout << j.serialize() << "\n";
}

int main(int argc, char const *argv[])
{
	std::cout << "============= Performance Test =============\n";

	testSpeed("test/data/citm_catalog.json");
	testSpeed("test/data/webxml.json", 1000);
	std::cout << "(Download citylots.json (185MB) from Github "
		"(zeMirco/sf-city-lots-json) and uncomment the following line.)\n";
	// testSpeed("test/data/citylots.json", 1);

	std::cout << "============= Error Handling Test =============\n";

	testErrorHandling("{");
	testErrorHandling("[");
	testErrorHandling("]");
	testErrorHandling("}");
	testErrorHandling("[}");
	testErrorHandling("[1, 2, ]");
	testErrorHandling("[1, 2");
	testErrorHandling("3e++5");
	testErrorHandling("3e309");
	testErrorHandling("[truk]");
	testErrorHandling("[fallse]");
	testErrorHandling("[nulll]");
	testErrorHandling("[\"hello]");
	testErrorHandling("[[1, [4, 5, [6] ,3]]");
	testErrorHandling("/ comment */  [1, 2, 3]");

	std::cout << "============= Serialization(Pretty Print) Test =============\n";

	testPrettyPrint("{\"number\":   [1,2,4,6,{\"string\":  \"foobar\"}]}");
	testPrettyPrint("{\"UTF8中文\":  \"内容\"}");


}

