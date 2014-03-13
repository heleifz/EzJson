#include "EzJson.h"

#include <iostream>
#include <fstream>
#include <string>

int main() {
	// remove spaces
	std::cout << "remove spaces" << std::endl;
	char tmp[] = "hello world my name is h e lei ";
	removeSpaces(tmp);
	std::cout << tmp << std::endl;

	// number
	std::cout << "number" << std::endl;
	const char* num = "23.33e+10";
	const char* end = num + strlen(num);
	std::cout << isNumber(num) << std::endl;
	std::cout << (num == end) << std::endl;
	
	// string
	std::cout << "string" << std::endl;
	const char* str = "\"hello\\b \\nworld\"";
	const char* endstr = str + strlen(str);
	std::cout << isString(str) << std::endl;
	std::cout << (str == endstr) << std::endl;

	//array
	std::cout << "array" << std::endl;
	const char* arr = "[1,2,[4,5.5,\"nihao\",7],{\"asdf\":3}]";
	const char* endarr = arr + strlen(arr);
	std::cout << isArray(arr) << std::endl;
	std::cout << (arr == endarr) << std::endl;

	//object
	std::cout << "object" << std::endl;
	const char* obj = "{\"hello\":[1,2,4],\"world\":\"nihao\"}";
	const char* endobj = obj + strlen(obj);
	std::cout << isObject(obj) << std::endl;
	std::cout << (obj == endobj) << std::endl;

	//with spaces
	// read entire file into string
	std::ifstream is("test.txt", std::ifstream::binary);
	if (is) {
		// get length of file:
		is.seekg(0, is.end);
		int length = is.tellg();
		is.seekg(0, is.beg);

		char *f = new char[length + 1];
		is.read(f, length);
		is.close();
		f[length] = '\0';

		std::cout << "file" << std::endl;
		removeSpaces(f);
		std::cout << "removed spaces: " << f << std::endl;

		const char* ff = (const char*)f;
		const char* endf = ff + strlen(ff);
		std::cout << isObject(ff) << std::endl;
		std::cout << (ff == endf) << std::endl;
	}
	else {
		std::cout << "Could not open test.txt\n";
	}

}