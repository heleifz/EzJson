#ifndef __EZ_JSON__
#define __EZ_JSON__

#include <sstream>
#include <vector>
#include <map>
#include <memory>

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
};

class NumberNode : public Node
{
public:
	void acceptVisitor(std::shared_ptr<Visitor> visitor)
	{
		visitor->visit(this);
	}
	NumberNode(const std::string& val) : value(val) {}
	virtual double asDouble()
	{
		return std::stod(value);
	}
	virtual std::string asString()
	{
		return value;
	}
	std::string value;
};

// string
class StringNode : public Node
{
public:
	StringNode(const std::string& val) : value(val) { }
	void acceptVisitor(std::shared_ptr<Visitor> visitor)
	{
		visitor->visit(this);
	}
	virtual std::string asString()
	{
		return value;
	}
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
	std::map<std::string, std::shared_ptr<Node>> pairs;
};

//////////////////////////////////////////////////////////////////
// a LL(1) json parser

class Parser
{
private:
	// primitive "eaters", they dont generate result
	static bool eatSymbol(const char*& str, char sym)
	{
		if (*str == sym)
		{
			str++;
			return true;
		}
		return false;
	}

	static void eatSpaces(const char*& str)
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
	static bool eatComment(const char*& str)
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

	static bool eatZeroDigit(const char*& str)
	{
		return eatSymbol(str, '0');
	}

	static bool eatNonZeroDigit(const char*& str)
	{
		if (*str <= '9' && *str > '0')
		{
			str++;
			return true;
		}
		return false;
	}

	static bool eatDigit(const char*& str)
	{
		if (*str <= '9' && *str >= '0')
		{
			str++;
			return true;
		}
		return false;
	}

	// number / string / true / false / null  -> return a "result"

	static bool eatNumber(const char*& str, std::string& result)
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

	static bool eatString(const char*& str, std::string& result)
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

	static bool eatNull(const char*& str)
	{
		if (eatSymbol(str, 'n') && eatSymbol(str, 'u') &&
			eatSymbol(str, 'l') && eatSymbol(str, 'l'))
		{
			return true;
		}
		return false;
	}

	// eat true / false

	static bool eatTrue(const char*& str)
	{
		return (eatSymbol(str, 't') && eatSymbol(str, 'r') &&
			eatSymbol(str, 'u') && eatSymbol(str, 'e'));
	}

	static bool eatFalse(const char*& str)
	{
		return (eatSymbol(str, 'f') && eatSymbol(str, 'a') &&
			eatSymbol(str, 'l') && eatSymbol(str, 's') && eatSymbol(str, 'e'));
	}

	// compound value : returns a tree

	static bool eatValue(const char*& str, std::shared_ptr<Node>& result)
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

	static bool eatArray(const char*& str, std::shared_ptr<Node>& result)
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
			ret->childs.push_back(one);
			eatSpaces(str);

			// ,value  ,value (trailling comma is not allowed)
			while (eatSymbol(str, ','))
			{
				if (!eatValue(str, one))
				{
					return false;
				}
				ret->childs.push_back(one);
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

	static bool eatObject(const char*& str, std::shared_ptr<Node>& result)
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
			ret->pairs.insert(std::make_pair(key, val));
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
				ret->pairs.insert(std::make_pair(key, val));
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
public:
	static std::shared_ptr<Node> parse(const std::string& str)
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
};

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
		stream << ptr->value;
	}
	void visit(TrueNode* ptr)
	{
		stream << "true";
	}
	void visit(FalseNode* ptr)
	{
		stream << "false";
	}
	void visit(StringNode* ptr)
	{
		stream << '"' << ptr->value << '"';
	}
	void visit(ArrayNode* ptr)
	{
		stream << "[";
		for (int i = 0; i != ptr->childs.size(); ++i)
		{
			if (ptr->childs[i])
			{
				ptr->childs[i]->acceptVisitor(shared_from_this());
			}
			else
			{
				stream << "null";
			}
			if (i != ptr->childs.size() - 1)
			{
				stream << ", ";
			}
		}
		stream << "]";
	}
	// 只有object才缩进
	void visit(ObjectNode* ptr)
	{
		// 换行 - 缩进 - 花括号
		stream << (indent == 0 ? "" : "\n") << std::string(indent, ' ') << "{" << std::endl;
		indent += 4;

		if (!ptr->pairs.empty())
		{
			auto i = ptr->pairs.begin();
			stream << std::string(indent, ' ') << '"' <<
				i->first << '"' << " : ";
			if (i->second)
			{
				i->second->acceptVisitor(shared_from_this());
			}
			i++;
			for (; i != ptr->pairs.end(); ++i)
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

#endif