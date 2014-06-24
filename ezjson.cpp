#include "ezjson.h"

#include "include/globals.h"
#include "include/text_scanner.h"
#include "include/parser.h"
#include "include/containers.h"
#include "include/allocator.h"

#include <memory>
#include <deque>

/**
 * AST Node
 */

struct Node
{
	virtual Node* at(size_t idx)
	{
		throw NotAnArrayError();
	}
	virtual Node* field(const char* k)
	{
		throw NotAnObjectError();
	}
	virtual size_t size()
	{
		throw NotAnArrayOrObjectError();
	}
	virtual std::vector<std::string> fields()
	{
		throw NotAnObjectError();
	}
	virtual double asDouble()
	{
		throw NotConvertibleError();
	}
	virtual bool asBool()
	{
		throw NotConvertibleError();
	}
	virtual std::string asString()
	{
		throw NotConvertibleError();
	}
	void* operator new(size_t sz, ChunkAllocator& alc)
	{
		return alc.alloc(sz);
	}
	void operator delete(void*, ChunkAllocator& alc)
	{
		// Do Nothing (let the chunk allocator to release the memory)
	}
};

struct NumberNode : public Node
{
	NumberNode(double val) : data(val) {}
	double asDouble()
	{
		return data;
	}
	double data;
};

struct StringNode : public Node
{
	StringNode(const char *b, const char *e, ChunkAllocator& alc)
	: data(b, e, alc) {}

	std::string asString()
	{
		return data.asSTLString();
	}

	String data;
};

struct BoolNode : public Node
{
	BoolNode(bool b) : data(b) {}
	bool asBool()
	{
		return data;
	}
	bool data;
};

struct NullNode : public Node
{
};

struct ArrayNode : public Node
{
	ArrayNode(ChunkAllocator& alloc)
	: data(alloc) {}

	Node* at(size_t idx)
	{
		return data[idx];
	}
	size_t size()
	{
		return data.size();
	}

	Array<Node*> data;
};

struct ObjectNode : public Node
{
	ObjectNode(ChunkAllocator& allocator)
	: data(allocator) {}

	Node* field(const char* k)
	{
		return data.get(k);
	}

	std::vector<std::string> fields()
	{
		return data.keys();
	}

	size_t size()
	{
		return data.size();
	}

	Dictionary<Node*> data;
};

/**
 * Parser callbacks
 */

class ASTBuildHandler
{
private:

	ChunkAllocator& allocator;
	std::deque<Node*> parseStack;

public:

	ASTBuildHandler(ChunkAllocator& a) : allocator(a)
	{
	}

	Node* getAST()
	{
		Node *ast = parseStack.front();
		parseStack.pop_front();
		return ast;
	}

	void stringAction(const char *b, const char *e)
	{
		parseStack.push_back(new (allocator) StringNode(b, e, allocator));
	}

	void numberAction(double val)
	{
		parseStack.push_back(new (allocator) NumberNode(val));
	}

	void boolAction(bool b)
	{
		parseStack.push_back(new (allocator) BoolNode(b));
	}

	void nullAction()
	{
		parseStack.push_back(new (allocator) NullNode());
	}

	void beginArrayAction() {}

	void endArrayAction(size_t size)
	{
		auto arr = new (allocator) ArrayNode(allocator);
		for (auto iter = parseStack.end() - size; iter != parseStack.end(); ++iter)
		{
			arr->data.pushBack(*iter, allocator);
		}
		parseStack.erase(parseStack.end() - size, parseStack.end());
		parseStack.push_back(arr);
	}

	void beginObjectAction() {}

	void endObjectAction(size_t size)
	{
		size *= 2;
		auto obj = new (allocator) ObjectNode(allocator);
		for (auto iter = parseStack.end() - size; iter != parseStack.end(); iter += 2)
		{
			obj->data.insert(static_cast<StringNode*>(*iter)->data, *(iter + 1), allocator);
		}
		parseStack.erase(parseStack.end() - size, parseStack.end());
		parseStack.push_back(obj);
	}
};

EzJSON::EzJSON(const char *content) : allocator(std::make_shared<ChunkAllocator>())
{
	ASTBuildHandler handler(*allocator);
	Parser<TextScanner, ASTBuildHandler> (TextScanner(content), handler).parseValue();
	node = handler.getAST();
}

EzJSON::EzJSON(Node* nd, std::shared_ptr<ChunkAllocator> alc) : node(nd), allocator(alc)
{
}

EzJSON EzJSON::at(size_t idx)
{
	return EzJSON(node->at(idx), allocator);
}

EzJSON EzJSON::field(const char *key)
{
	return EzJSON(node->field(key), allocator);
}

double EzJSON::asDouble() const
{
	return node->asDouble();
}

bool EzJSON::asBool() const
{
	return node->asBool();
}

std::string EzJSON::asString() const
{
	return node->asString();
}

size_t EzJSON::size() const
{
	return node->size();
}

std::vector<std::string> EzJSON::fields() const
{
	return node->fields();
}
