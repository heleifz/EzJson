#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "EzJson.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <ctime>

template <class STREAM>
class PrintVisitor : public Visitor {
private:
	int indent;
	STREAM& stream;
public:
	// initial indention is 0
	PrintVisitor(STREAM& s) : stream(s), indent(0) {}

	void visit(NumberNode* ptr) {
		stream << ptr->value;
	}
	void visit(BoolNode* ptr) {
		stream << (ptr->value ? "true" : "false");
	}
	void visit(StringNode* ptr) {
		stream << '"' << ptr->value << '"';
	}
	void visit(ArrayNode* ptr) {
		stream << "[";
		for (int i = 0; i != ptr->childs.size(); ++i) {
			if (ptr->childs[i]) {
				ptr->childs[i]->acceptVisitor(shared_from_this());
			}
			else {
				stream << "null";
			}
			if (i != ptr->childs.size() - 1) {
				stream << ", ";
			}
		}
		stream << "]";
	}
	// 只有object才缩进
	void visit(ObjectNode* ptr) {
		// 换行 - 缩进 - 花括号
		stream << (indent == 0 ? "" : "\n") << std::string(indent, ' ') << "{" << std::endl;
		indent += 4;

		if (!ptr->pairs.empty()) {
			auto i = ptr->pairs.begin();
			stream << std::string(indent, ' ') << '"' << i->first << '"' << " : ";
			if (i->second) {
				i->second->acceptVisitor(shared_from_this());
			}
			i++;
			for (; i != ptr->pairs.end(); ++i) {
				stream << ",\n";
				stream << std::string(indent, ' ') << '"' << i->first << '"' << " : ";
				if (i->second) {
					i->second->acceptVisitor(shared_from_this());
				}
				else {
					stream << "null";
				}
			}
		}
		indent -= 4;
		stream << std::endl << std::string(indent, ' ') << "}";
	}
};


int main() {

	// detect memory leak
	{
		// read entire file into string
		std::ifstream is("test5.txt");
		std::ofstream os("testout.txt");
		if (is) {
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
		else {
			std::cout << "Could not open test.txt\n";
		}
	}
	_CrtDumpMemoryLeaks();

}