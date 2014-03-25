#ifndef __EZ_JSON__
#define __EZ_JSON__

#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>

namespace __JSONDetail
{

class NumberNode;
class StringNode;
class TrueNode;
class FalseNode;
class ArrayNode;
class ObjectNode;

class Visitor : public std::enable_shared_from_this<Visitor>
{
public:
	virtual void visit(NumberNode* ptr) = 0;
	virtual void visit(TrueNode* ptr) = 0;
	virtual void visit(FalseNode* ptr) = 0;
	virtual void visit(StringNode* ptr) = 0;
	virtual void visit(ArrayNode* ptr) = 0;
	virtual void visit(ObjectNode* ptr) = 0;
};

class NotAnObjectError {};
class NotAnArrayError {};
class NotConvertibleError {};

// abstract syntax tree

class Node
{
public:
	virtual ~Node() { }
	virtual void acceptVisitor(std::shared_ptr<Visitor> visitor) = 0;
	virtual std::shared_ptr<Node> field(const std::string& key)
	{
		throw NotAnObjectError();
	}
	virtual std::shared_ptr<Node> at(int idx)
	{
		throw NotAnArrayError();
	}
	virtual double asDouble()
	{
		throw NotConvertibleError();
	}
	virtual std::string asString()
	{
		throw NotConvertibleError();
	}
	virtual bool asBool()
	{
		throw NotConvertibleError();
	}
	virtual std::vector<std::shared_ptr<Node>>& asVectorRef() {
		throw NotConvertibleError();
	}
	virtual std::map<std::string, std::shared_ptr<Node>>& asMapRef() {
		throw NotConvertibleError();
	}
};

class NumberNode : public Node
{
public:
	void acceptVisitor(std::shared_ptr<Visitor> visitor)
	{
		visitor->visit(this);
	}
	explicit NumberNode(const std::string& val) : value(val) {}
	virtual double asDouble()
	{
		return std::stod(value);
	}
	virtual std::string asString()
	{
		return value;
	}
private:
	std::string value;
};

// string
class StringNode : public Node
{
public:
	explicit StringNode(const std::string& val) : value(val) { }
	void acceptVisitor(std::shared_ptr<Visitor> visitor)
	{
		visitor->visit(this);
	}
	virtual std::string asString()
	{
		return value;
	}
private:
	std::string value;
};

// true / false
class TrueNode : public Node
{
public:
	void acceptVisitor(std::shared_ptr<Visitor> visitor)
	{
		visitor->visit(this);
	}
	virtual double asDouble()
	{
		return 1.0;
	}
	virtual std::string asString()
	{
		return "true";
	}
	virtual bool asBool()
	{
		return true;
	}
};

class FalseNode : public Node
{
public:
	void acceptVisitor(std::shared_ptr<Visitor> visitor)
	{
		visitor->visit(this);
	}
	virtual double asDouble()
	{
		return 0.0;
	}
	virtual std::string asString()
	{
		return "false";
	}
	virtual bool asBool()
	{
		return false;
	}
};

// **************************
// null node is a "nullptr"
// **************************

// use vector for array

class ArrayNode : public Node
{
public:
	ArrayNode() {}
	virtual std::shared_ptr<Node> at(int idx)
	{
		return childs[idx];
	}
	void acceptVisitor(std::shared_ptr<Visitor> visitor)
	{
		visitor->visit(this);
	}
	virtual std::vector<std::shared_ptr<Node>>& asVectorRef() {
		return childs;
	}
private:
	std::vector<std::shared_ptr<Node>> childs;
};

class ObjectNode : public Node
{
public:
	virtual std::shared_ptr<Node> field(const std::string& key)
	{
		return pairs.at(key);
	}
	void acceptVisitor(std::shared_ptr<Visitor> visitor)
	{
		visitor->visit(this);
	}
	virtual std::map<std::string, std::shared_ptr<Node>>& asMapRef() {
		return pairs;
	}
private:
	std::map<std::string, std::shared_ptr<Node>> pairs;
};

//////////////////////////////////////////////////////////////////

// a recursive descending json parser

namespace Parser
{
	// primitive "eaters", they dont generate result
	bool eatSymbol(const char*& str, char sym)
	{
		if (*str == sym)
		{
			str++;
			return true;
		}
		return false;
	}

	bool eatComment(const char*& str);
	void eatSpaces(const char*& str)
	{
		while (*str == '\n' || *str == '\t' ||
			*str == '\r' || *str == ' ')
		{
			str++;
		}
		// comments must be at spaces
		eatComment(str);
	}

	// support c-style one line or multi-line comment
	bool eatComment(const char*& str)
	{
		// comment start
		if (eatSymbol(str, '/'))
		{
			if (eatSymbol(str, '/'))
			{
				// one line comment
				while (*str != '\n' && *str != '\0')
				{
					str++;
				}
				eatSpaces(str);
			}
			else if (eatSymbol(str, '*'))
			{
				// multi-line comment
				while (*str != '\0')
				{
					// reach the end of comment
					if (*str == '*' && *(str + 1) == '/')
					{
						str += 2;
						break;
					}
					str++;
				}
				eatSpaces(str);
			}
			else
			{
				// not a comment
				return false;
			}
		}
		return true;
	}

	bool eatZeroDigit(const char*& str)
	{
		return eatSymbol(str, '0');
	}

	bool eatNonZeroDigit(const char*& str)
	{
		if (*str <= '9' && *str > '0')
		{
			str++;
			return true;
		}
		return false;
	}

	bool eatDigit(const char*& str)
	{
		if (*str <= '9' && *str >= '0')
		{
			str++;
			return true;
		}
		return false;
	}

	// number / string / true / false / null  -> return a "result"

	bool eatNumber(const char*& str, std::string& result)
	{
		const char* start = str;

		// optional minus sign
		eatSymbol(str, '-');

		// zero, or more than zero digits (cannot start with 0)
		if (!eatZeroDigit(str))
		{
			if (!eatNonZeroDigit(str))
			{
				return false;
			}
			while (eatDigit(str));
		}

		// real number
		if (eatSymbol(str, '.'))
		{
			// at least one
			if (!eatDigit(str))
			{
				return false;
			}
			// eat rest
			while (eatDigit(str));
		}

		// scientific notation
		if (eatSymbol(str, 'e') || eatSymbol(str, 'E'))
		{
			// eat optional +/- sign
			(eatSymbol(str, '+') || eatSymbol(str, '-'));
			if (!eatDigit(str))
			{
				return false;
			}
			// eat rest
			while (eatDigit(str));
		}
		result = std::string(start, str);
		return true;
	}

	// eat string

	bool eatString(const char*& str, std::string& result)
	{
		const char* start = str;
		const char table[] =
		{
			'"', '\\', '/', 'b', 'f', 'n', 'r', 't', 'u'
		};
		if (!eatSymbol(str, '"'))
		{
			return false;
		}
		// look ahead
		while (*str != '"')
		{
			// end of str, format error.
			if (*str == '\0')
			{
				return false;
			}
			else if (eatSymbol(str, '\\'))
			{
				int i = 0;
				for (; i < 8; ++i)
				{
					if (eatSymbol(str, table[i]))
					{
						break;
					}
				}
				// special char found
				if (i < 8)
				{
					continue;
				}
				// eat unicode
				else if (eatSymbol(str, table[8]))
				{
					if (!(eatDigit(str) && eatDigit(str) && eatDigit(str) && eatDigit(str)))
					{
						return false;
					}
					continue;
				}
				else
				{
					return false;
				}
			}
			else
			{
				// eat anything else
				str++;
			}
		}
		if (!eatSymbol(str, '"'))
		{
			return false;
		}

		// strip quotes
		result = std::string(start + 1, str - 1);

		return true;
	}

	// eat null

	bool eatNull(const char*& str)
	{
		if (eatSymbol(str, 'n') && eatSymbol(str, 'u') &&
			eatSymbol(str, 'l') && eatSymbol(str, 'l'))
		{
			return true;
		}
		return false;
	}

	// eat true / false

	bool eatTrue(const char*& str)
	{
		return (eatSymbol(str, 't') && eatSymbol(str, 'r') &&
			eatSymbol(str, 'u') && eatSymbol(str, 'e'));
	}

	bool eatFalse(const char*& str)
	{
		return (eatSymbol(str, 'f') && eatSymbol(str, 'a') &&
			eatSymbol(str, 'l') && eatSymbol(str, 's') && eatSymbol(str, 'e'));
	}

	// compound value : returns a tree

	bool eatArray(const char*& str, std::shared_ptr<Node>& result);
	bool eatObject(const char*& str, std::shared_ptr<Node>& result);
	bool eatValue(const char*& str, std::shared_ptr<Node>& result)
	{
		eatSpaces(str);
		std::string val;
		if (eatNumber(str, val))
		{
			result = std::make_shared<NumberNode>(val);
			return true;
		}
		else if (eatString(str, val))
		{
			result = std::make_shared<StringNode>(val);
			return true;
		}
		else if (eatObject(str, result))
		{
			return true;
		}
		else if (eatArray(str, result))
		{
			return true;
		}
		else if (eatFalse(str))
		{
			result = std::make_shared<FalseNode>();
			return true;
		}
		else if (eatTrue(str))
		{
			result = std::make_shared<TrueNode>();
			return true;
		}
		else if (eatNull(str))
		{
			result = std::shared_ptr<Node>(nullptr);
			return true;
		}
		return false;
	}

	bool eatArray(const char*& str, std::shared_ptr<Node>& result)
	{
		std::shared_ptr<ArrayNode> ret = std::make_shared<ArrayNode>();

		if (!eatSymbol(str, '['))
		{
			return false;
		}
		eatSpaces(str);
		// allow empty array
		if (*str != ']')
		{
			std::shared_ptr<Node> one;
			// value ....
			if (!eatValue(str, one))
			{
				return false;
			}
			ret->asVectorRef().push_back(one);
			eatSpaces(str);

			// ,value  ,value (trailling comma is not allowed)
			while (eatSymbol(str, ','))
			{
				if (!eatValue(str, one))
				{
					return false;
				}
				ret->asVectorRef().push_back(one);
				eatSpaces(str);
			}
		}
		if (!eatSymbol(str, ']'))
		{
			return false;
		}

		result = ret;

		return true;
	}

	bool eatObject(const char*& str, std::shared_ptr<Node>& result)
	{
		std::shared_ptr<ObjectNode> ret = std::make_shared<ObjectNode>();

		if (!eatSymbol(str, '{'))
		{
			return false;
		}
		eatSpaces(str);
		// look ahead
		if (*str != '}')
		{
			std::string key;
			std::shared_ptr<Node> val;

			eatSpaces(str);
			bool good = false;
			if (eatString(str, key))
			{
				eatSpaces(str);
				if (eatSymbol(str, ':'))
				{
					good = eatValue(str, val);
				}
			}
			if (!good)
			{
				return false;
			}
			ret->asMapRef().insert(std::make_pair(key, val));
			// , str : val  , str : val ...
			eatSpaces(str);
			while (eatSymbol(str, ','))
			{
				eatSpaces(str);
				good = false;
				if (eatString(str, key))
				{
					eatSpaces(str);
					if (eatSymbol(str, ':'))
					{
						good = eatValue(str, val);
					}
				}
				if (!good)
				{
					return false;
				}
				ret->asMapRef().insert(std::make_pair(key, val));
			}
		}
		eatSpaces(str);
		if (!eatSymbol(str, '}'))
		{
			return false;
		}
		result = ret;
		return true;
	}
	std::shared_ptr<Node> parse(const std::string& str)
	{
		std::shared_ptr<Node> ret;
		const char* ptr = str.c_str();
		if (eatValue(ptr, ret))
		{
			return ret;
		}
		else
		{
			return nullptr;
		}
	}
}

template <class STREAM>
class PrintVisitor : public Visitor
{
private:
	int indent;
	STREAM& stream;
public:
	// initial indention is 0
	PrintVisitor(STREAM& s) : stream(s), indent(0) {}

	void visit(NumberNode* ptr)
	{
		stream << ptr->asString();
	}
	void visit(TrueNode* ptr)
	{
		stream << ptr->asString();
	}
	void visit(FalseNode* ptr)
	{
		stream << ptr->asString();
	}
	void visit(StringNode* ptr)
	{
		stream << '"' << ptr->asString() << '"';
	}
	void visit(ArrayNode* ptr)
	{
		stream << "[";
		auto& vec = ptr->asVectorRef();
		for (int i = 0; i != vec.size(); ++i)
		{
			if (vec[i])
			{
				vec[i]->acceptVisitor(shared_from_this());
			}
			else
			{
				stream << "null";
			}
			if (i != vec.size() - 1)
			{
				stream << ", ";
			}
		}
		stream << "]";
	}
	void visit(ObjectNode* ptr)
	{
		stream << (indent == 0 ? "" : "\n") << std::string(indent, ' ') << "{" << std::endl;
		indent += 4;

		auto& mp = ptr->asMapRef();

		if (!mp.empty())
		{
			auto i = mp.begin();
			stream << std::string(indent, ' ') << '"' <<
				i->first << '"' << " : ";
			if (i->second)
			{
				i->second->acceptVisitor(shared_from_this());
			}
			i++;
			for (; i != mp.end(); ++i)
			{
				stream << ",\n";
				stream << std::string(indent, ' ') << '"' << i->first << '"' << " : ";
				if (i->second)
				{
					i->second->acceptVisitor(shared_from_this());
				}
				else
				{
					stream << "null";
				}
			}
		}
		indent -= 4;
		stream << std::endl << std::string(indent, ' ') << "}";
	}
};

// fluent interface

class JSON {
public:
	// static factory functions
	template <typename T>
	static JSON fromNumber(T num) {
		std::stringstream ss;
		ss << num;
		auto n = std::make_shared<NumberNode>(ss.str());
		return JSON(n);
	}
	static JSON fromBool(bool b)
	{
		if (b)
		{
			return JSON(std::make_shared<TrueNode>());
		}
		else
		{
			return JSON(std::make_shared<FalseNode>());
		}
	}
	static JSON fromString(const std::string& str)
	{
		return JSON(std::make_shared<StringNode>(str));
	}
	static JSON makeObject()
	{
		return JSON(std::make_shared<ObjectNode>());
	}
	static JSON makeArray()
	{
		return JSON(std::make_shared<ArrayNode>());
	}

	// construct from file
	explicit JSON(const char *path) {
		std::ifstream is(path);
		auto in = std::istreambuf_iterator<char>(is);
		std::string s(in, std::istreambuf_iterator<char>());;
		node = Parser::parse(s);
	}
	JSON field(const std::string& key) const
	{
		return JSON(node->field(key));
	}
	JSON at(int idx) const
	{
		return JSON(node->at(idx));
	}
	double asDouble() const
	{
		return node->asDouble();
	}
	std::string asString() const
	{
		std::stringstream ss;
		auto v = std::make_shared<PrintVisitor<std::stringstream>>(ss);
		node->acceptVisitor(v);
		return ss.str();
	}
	bool asBool() const
	{
		return node->asBool();
	}
	JSON& append(const JSON& child)
	{
		node->asVectorRef().push_back(child.node);
		return *this;
	}
	JSON& set(int idx, const JSON& child)
	{
		node->asVectorRef()[idx] = child.node;
		return *this;
	}
	JSON& set(const std::string& field, const JSON& value)
	{
		node->asMapRef()[field] = value.node;
		return *this;
	}
	std::vector<std::string> allFields() const
	{
		auto& mp = node->asMapRef();
		std::vector<std::string> result;
		for (auto i = mp.begin(); i != mp.end(); ++i)
		{
			result.push_back(i->first);
		}
		return result;
	}
	int arraySize() {
		return node->asVectorRef().size();
	}
private:
	JSON(std::shared_ptr<Node> data) : node(data) {}
	std::shared_ptr<Node> node;
};

}

using __JSONDetail::JSON;

template <typename STREAM>
STREAM& operator<<(STREAM& s, const JSON& json)
{
	s << json.asString();
	return s;
}

#endif