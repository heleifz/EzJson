#ifndef __EZ_JSON__
#define __EZ_JSON__

// JSON document object

#include "include/parser.h"

#include <memory>
#include <deque>
#include <string>

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
		return std::string(data.begin(), data.end());
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
	size_t size()
	{
		return data.size();
	}

	Dictionary<Node*> data;
};

class JSON {
public:
	void stringAction(const char *b, const char *e)
	{
		parseStack.push_back(new (*allocator) StringNode(b, e, *allocator));
	}
	void numberAction(double val)
	{
		parseStack.push_back(new (*allocator) NumberNode(val));
	}
	void boolAction(bool b)
	{
		parseStack.push_back(new (*allocator) BoolNode(b));
	}
	void nullAction()
	{
		parseStack.push_back(new (*allocator) NullNode());
	}
	void beginArrayAction() {}
	void endArrayAction(size_t size)
	{
		auto arr = new (*allocator) ArrayNode(*allocator);
		for (auto iter = parseStack.end() - size; iter != parseStack.end(); ++iter)
		{
			arr->data.pushBack(*iter, *allocator);
		}
		parseStack.erase(parseStack.end() - size, parseStack.end());
		parseStack.push_back(arr);
	}
	void beginObjectAction() {}
	void endObjectAction(size_t size)
	{
		size *= 2;
		auto obj = new (*allocator) ObjectNode(*allocator);
		for (auto iter = parseStack.end() - size; iter != parseStack.end(); iter += 2)
		{
			obj->data.insert(static_cast<StringNode*>(*iter)->data, *(iter + 1), *allocator);
		}
		parseStack.erase(parseStack.end() - size, parseStack.end());
		parseStack.push_back(obj);
	}

	explicit JSON(const char *content)
		: allocator(std::make_shared<ChunkAllocator>())
	{
		Parser<JSON>(Scanner(content), *this).parseValue();
		node = parseStack.front();
		parseStack.pop_back();
	}

	// fluent interface
	JSON at(size_t idx)
	{
		return JSON(node->at(idx), allocator);
	}
	JSON field(const char *key)
	{
		return JSON(node->field(key), allocator);
	}
	double asDouble()
	{
		return node->asDouble();
	}
	bool asBool()
	{
		return node->asBool();
	}
	std::string asString()
	{
		return node->asString();
	}
	size_t size()
	{
		return node->size();
	}

private:
	JSON(Node* nd, std::shared_ptr<ChunkAllocator> alc)
		: node(nd), allocator(alc)
	{}

	std::deque<Node*> parseStack;
	std::shared_ptr<ChunkAllocator> allocator;
	Node *node;
};

#endif