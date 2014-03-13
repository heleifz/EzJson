#include "EzJson.h"
#include <iostream>

int main() {
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
}