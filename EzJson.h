#ifndef __EZ_JSON__
#define __EZ_JSON__

#include "include/parser.h"

#include <string>
#include <iostream>

struct Node;
class MemoryPool;

// fluent interface
class JSON {
public:
	explicit JSON(const char *content)
	{
		DefaultAction a;
		Parser<>(Scanner(content), a).parseValue();
		std::cout << "total string : " << a.count << "\n";
	}
private:
	Node* node;
	std::shared_ptr<MemoryPool> pool;
};

#endif