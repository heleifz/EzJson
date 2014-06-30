#include "ezjson.h"

#include "include/globals.h"
#include "include/text_scanner.h"
#include "include/parser.h"
#include "include/allocator.h"
#include "include/containers.h"

#include <sstream>

namespace Ez
{

/**
 * AST Node
 */

class Node
{
public:

	virtual void serialize(std::stringstream& ss, size_t indentLevel = 0) const = 0;

	virtual Node* at(size_t) const
	{
		throw NotAnArrayError();
	}

	virtual Node* key(const char*) const
	{
		throw NotAnObjectError();
	}

	virtual void append(Node*)
	{
		throw NotAnArrayError();
	}

	virtual void setAt(size_t, Node*)
	{
		throw NotAnArrayError();
	}

	virtual void setKey(const char*, Node*)
	{
		throw NotAnObjectError();
	}

	virtual void removeAt(size_t)
	{
		throw NotAnArrayError();
	}

	virtual void removeKey(const char*)
	{
		throw NotAnObjectError();
	}

	virtual size_t size() const
	{
		throw NotAnArrayOrObjectError();
	}

	virtual std::vector<std::string> fields() const
	{
		throw NotAnObjectError();
	}

	virtual double asDouble() const
	{
		throw NotConvertibleError();
	}

	virtual bool asBool() const
	{
		throw NotConvertibleError();
	}

	virtual std::string asString() const
	{
		throw NotConvertibleError();
	}

	void* operator new(size_t sz, FastAllocator& alc)
	{
		return alc.alloc(sz);
	}

	void operator delete(void*, FastAllocator&)
	{
		// Do Nothing (let the allocator to release the memory)
	}
};

class NumberNode : public Node
{
private:

	double data;

public:

	NumberNode(double val) : data(val) {}

	virtual void serialize(std::stringstream& ss, size_t) const
	{
		ss << data;
	}

	double asDouble() const
	{
		return data;
	}
};

class StringNode : public Node
{
private:

	String data;
	friend class ASTBuildHandler;

public:

	StringNode(const char *b, const char *e, FastAllocator& alc)
		: data(b, e, alc) {}

	virtual void serialize(std::stringstream& ss, size_t) const
	{
		ss << '"' << asString() << '"';
	}

	std::string asString() const
	{
		return data.asSTLString();
	}
};

class BoolNode : public Node
{
private:

	bool data;

public:

	BoolNode(bool b) : data(b) {}
	virtual void serialize(std::stringstream& ss, size_t) const
	{
		if (data)
		{
			ss << "true";
		}
		else
		{
			ss << "false";
		}
	}

	bool asBool() const
	{
		return data;
	}
};

class NullNode : public Node
{
public:

	virtual void serialize(std::stringstream& ss, size_t) const
	{
		ss << "null";
	}
};

class ArrayNode : public Node
{
private:

	Array<Node*, FastAllocator> data;
	friend class ASTBuildHandler;

public:

	ArrayNode(FastAllocator& alloc)
		: data(alloc) {}

	virtual void serialize(std::stringstream& ss, size_t indentLevel) const
	{
		ss << "[";
		for (size_t i = 1; i < data.size(); ++i)
		{
			data[i - 1]->serialize(ss, indentLevel);
			ss << ", ";
		}
		if (data.size() > 0)
		{
			data[size() - 1]->serialize(ss, indentLevel);
		}
		ss << "]";
	}

	Node* at(size_t idx) const
	{
		return data[idx];
	}
	size_t size() const
	{
		return data.size();
	}

	void setAt(size_t idx, Node *node)
	{
		data[idx] = node;
	}

	void removeAt(size_t idx)
	{
		data.remove(idx);
	}

	void append(Node* node)
	{
		data.pushBack(node);
	}
};

class ObjectNode : public Node
{
private:

	Dictionary<Node*, FastAllocator> data;
	friend class ASTBuildHandler;

public:

	ObjectNode(FastAllocator& allocator)
		: data(allocator) {}

	virtual void serialize(std::stringstream& ss, size_t indentLevel) const
	{
		if (indentLevel > 0)
		{
			ss << "\n";
		}
		printIndent(ss, indentLevel);
		ss << "{\n";
		for (auto i = data.begin(); i < data.end() - 1; ++i)
		{
			printIndent(ss, indentLevel + 1);
			ss << '"' << i->first.asSTLString() << "\" : ";
			i->second->serialize(ss, indentLevel + 1);
			ss << ",\n";
		}
		if (data.size() > 0)
		{
			auto last = data.end() - 1;
			printIndent(ss, indentLevel + 1);
			ss << '"' << last->first.asSTLString() << "\" : ";
			last->second->serialize(ss, indentLevel + 1);
		}
		ss << "\n";
		printIndent(ss, indentLevel);
		ss << "}";
	}

	Node* key(const char* k) const
	{
		return data.get(k);
	}

	std::vector<std::string> fields() const
	{
		return data.keys();
	}

	size_t size() const
	{
		return data.size();
	}

	void setKey(const char *key, Node *node)
	{
		data.set(key, node);
	}

	void removeKey(const char *k)
	{
		data.remove(k);
	}

	void printIndent(std::stringstream& ss, size_t indentLevel) const
	{
		for (size_t i = 0; i < indentLevel; i++)
		{
			ss << "    ";
		}
	}
};

/**
 * Parser callbacks
 */

class ASTBuildHandler : public INonCopyable
{
private:

	FastAllocator& allocator;
	Array<Node*, FastAllocator> parseStack;

public:

	ASTBuildHandler(FastAllocator& a) : allocator(a), parseStack(a)
	{
	}

	Node* getAST()
	{
		if (parseStack.size() == 1)
		{
			return parseStack.popBack();
		}
		else
		{
			return nullptr;
		}
	}

	void stringAction(const char *b, const char *e)
	{
		parseStack.pushBack(new (allocator)StringNode(b, e, allocator));
	}

	void numberAction(double val)
	{
		parseStack.pushBack(new (allocator)NumberNode(val));
	}

	void boolAction(bool b)
	{
		parseStack.pushBack(new (allocator)BoolNode(b));
	}

	void nullAction()
	{
		parseStack.pushBack(new (allocator)NullNode());
	}

	void beginArrayAction() {}

	void endArrayAction(size_t size)
	{
		auto arr = new (allocator)ArrayNode(allocator);
		auto last = parseStack.end();
		for (auto iter = parseStack.end() - size; iter != last; ++iter)
		{
			arr->data.pushBack(*iter);
		}
		parseStack.shrink(size);
		parseStack.pushBack(arr);
	}

	void beginObjectAction() {}

	void endObjectAction(size_t size)
	{
		size *= 2;
		auto obj = new (allocator)ObjectNode(allocator);
		auto last = parseStack.end();
		for (auto iter = parseStack.end() - size; iter != last; iter += 2)
		{
			obj->data.set(static_cast<StringNode*>(*iter)->data, *(iter + 1));
		}
		parseStack.shrink(size);
		parseStack.pushBack(obj);
	}
};

JSON::JSON(const char *content)
	: allocator(std::make_shared<FastAllocator>())
{
	node = parse(content, *allocator);
}

JSON::JSON(Node* nd, std::shared_ptr<FastAllocator> alc)
	: node(nd), allocator(alc)
{
}

JSON JSON::at(size_t idx) const
{
	return JSON(node->at(idx), allocator);
}

JSON JSON::key(const char *key) const
{
	return JSON(node->key(key), allocator);
}

double JSON::asDouble() const
{
	return node->asDouble();
}

bool JSON::asBool() const
{
	return node->asBool();
}

std::string JSON::asString() const
{
	return node->asString();
}

size_t JSON::size() const
{
	return node->size();
}

std::vector<std::string> JSON::keys() const
{
	return node->fields();
}

std::string JSON::serialize() const
{
	std::stringstream ss;
	node->serialize(ss);
	return ss.str();
}

void JSON::append(const char* content)
{
	node->append(parse(content, *allocator));
}

void JSON::setAt(size_t idx, const char *content)
{
	node->setAt(idx, parse(content, *allocator));
}

void JSON::setKey(const char *k, const char *content)
{
	node->setKey(k, parse(content, *allocator));
}

void JSON::removeAt(size_t idx)
{
	node->removeAt(idx);
}

void JSON::removeKey(const char *k)
{
	node->removeKey(k);
}

Node* JSON::parse(const char *content, FastAllocator& alc) const
{
	ASTBuildHandler handler(alc);
	Parser<TextScanner, ASTBuildHandler>(TextScanner(content), handler).parseValue();
	Node *node = handler.getAST();
	if (!node)
	{
		throw ParseError("JSON string is empty.");
	}
	return node;
}

}
