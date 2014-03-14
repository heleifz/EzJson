#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "EzJson.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>

class PrintVisitor : public Visitor {
public:
	void visit(NumberNode* ptr) {
		std::cout << "number node : " << ptr->value << "\n";
	}
	void visit(BoolNode* ptr) {
		std::cout << "bool node : " << ptr->value << "\n";
	}
	void visit(StringNode* ptr) {
		std::cout << "string node : " << ptr->value << "\n";
	}
	void visit(ArrayNode* ptr) {
		std::cout << "array node (visited), size : " << ptr->childs.size() << "\n";
	}
	void visit(ObjectNode* ptr) {
		std::cout << "object node (visited), size : " << ptr->pairs.size() << "\n";
	}
};


int main() {

	// detect memory leak
	{
		// read entire file into string
		std::ifstream is("test2.txt");
		if (is) {
			std::string str((std::istreambuf_iterator<char>(is)),
				std::istreambuf_iterator<char>());
			std::shared_ptr<Node> result = Parser::parse(str);
			std::cout << "file" << std::endl;
			std::cout << "=============== visitor test ======================\n";
			std::shared_ptr<Visitor> v = std::make_shared<PrintVisitor>();
			result->acceptVisitor(v);
		}
		else {
			std::cout << "Could not open test.txt\n";
		}
	}
	_CrtDumpMemoryLeaks();

}