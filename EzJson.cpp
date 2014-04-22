#include <sstream>
#include <fstream>
#include <vector>
#include <map>

#include "EzJson.h"

class ScanError {};
class ParseError {};
class NotAnObjectError {};
class NotAnArrayError {};
class NotConvertibleError {};

class Scanner
{
public:
	Scanner(const char* inp)
		: tokenBegin(inp), tokenEnd(inp)
	{
		next();
	}
	void next()
	{
		char state = START;
		tokenBegin = tokenEnd;
		while (1)
		{
			char current = *tokenEnd;
			if (current == '\0')
			{
				type = EOS;
				return;
			}
			tokenEnd++;
			switch (state)
			{
			case START:
				if (current == ' ' || current == '\n' ||
					current == '\t' || current == '\r')
				{
					tokenBegin++;
				}
				else if (current == 't')
				{
					if (*tokenEnd != 'r' || *(tokenEnd + 1) != 'u' ||
						*(tokenEnd + 2) != 'e')
					{
						throw ScanError();
					}
					tokenEnd += 3;
					type = TRU;
					return;
				}
				else if (current == 'f')
				{
					if (*tokenEnd != 'a' || *(tokenEnd + 1) != 'l' ||
						*(tokenEnd + 2) != 's' || *(tokenEnd + 3) != 'e')
					{
						throw ScanError();
					}
					tokenEnd += 4;
					type = FAL;
					return;
				}
				else if (current == 'n')
				{
					if (*tokenEnd != 'u' || *(tokenEnd + 1) != 'l' ||
						*(tokenEnd + 2) != 'l')
					{
						throw ScanError();
					}
					tokenEnd += 3;
					type = NUL;
					return;
				}
				else if (current == '"')
				{
					state = STRINGCONTENT;
				}
				else if (current == '/')
				{
					state = SLASH;
				}
				else if (current == '-' || isDigit(current))
				{
					state = NUMCONTENT;
				}
				else
				{
					if (current != '{' && current != '}' &&
						current != '[' && current != ']' &&
						current != ':' && current != ',')
					{
						throw ScanError();
					}
					type = (TokenType)current;
					return;
				}
				break;
			case NUMCONTENT:
				if (!(current == 'e' || current == 'E' ||
					current == '+' || current == '-' ||
					current == '.' || isDigit(current)))
				{
					// backup
					tokenEnd--;
					type = NUM;
					return;
				}
				break;
			case STRINGCONTENT:
				if (current == '\\')
				{
					state = ESCAPE;
				}
				else if (current == '"')
				{
					type = STR;
					return;
				}
				break;
			case ESCAPE:
				state = STRINGCONTENT;
				break;
			case SLASH:
				if (current == '/')
				{
					state = LINECOMMENT;
				}
				else if (current == '*')
				{
					state = BLOCKCOMMENT;
				}
				else
				{
					throw ScanError();
				}
				break;
			case LINECOMMENT:
				if (current == '\n')
				{
					// skip comment
					tokenBegin = tokenEnd;
					state = START;
				}
				break;
			case BLOCKCOMMENT:
				if (current == '*')
				{
					state = STAR;
				}
				break;
			case STAR:
				if (current == '/')
				{
					// skip comment
					tokenBegin = tokenEnd;
					state = START;
				}
				else if (current != '*')
				{
					state = BLOCKCOMMENT;
				}
				break;
			}
		}
	}
	TokenType lookahead()
	{
		return type;
	}
	std::string getText()
	{
		return std::string(tokenBegin, tokenEnd);
	}
	void match(TokenType t)
	{
		if (t == type)
		{
			next();
		}
		else
		{
			throw ParseError();
		}
	}
	void match(TokenType t, std::string& out)
	{
		if (t == type)
		{
			out = std::string(tokenBegin, tokenEnd);
			next();
		}
		else
		{
			throw ParseError();
		}
	}
private:
	enum DFAState
	{
		START = 0,
		NUMCONTENT,
		STRINGCONTENT,
		ESCAPE,
		SLASH,
		LINECOMMENT,
		BLOCKCOMMENT,
		STAR
	};
	bool isDigit(char ch)
	{
		ch -= '0';
		return (ch >= 0 && ch <= 9);
	}
	TokenType type;
	const char* tokenBegin;
	const char* tokenEnd;
};

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
	explicit NumberNode(const std::string& val) : value(val) {}
	void acceptVisitor(std::shared_ptr<Visitor> visitor)
	{
		visitor->visit(this);
	}
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
	explicit StringNode(const std::string& val) : value(val) {}
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

class Parser {
public:
	Parser(const Scanner& sc)
		: scanner(sc)
	{ }
	std::shared_ptr<Node> parseNumber()
	{
		std::string val;
		scanner.match(NUM, val);
		return std::make_shared<NumberNode>(val);
	}
	std::shared_ptr<Node> parseTrue()
	{
		scanner.match(TRU);
		return std::make_shared<TrueNode>();
	}
	std::shared_ptr<Node> parseFalse()
	{
		scanner.match(FAL);
		return std::make_shared<FalseNode>();
	}
	std::shared_ptr<Node> parseNull()
	{
		scanner.match(NUL);
		return nullptr;
	}
	std::shared_ptr<Node> parseString()
	{
		std::string val;
		scanner.match(STR, val);
		return std::make_shared<StringNode>(
			std::string(val.begin() + 1, val.end() - 1));
	}
	std::shared_ptr<Node> parseValue()
	{
		TokenType tk = scanner.lookahead();
		std::string val;
		switch (tk)
		{
		case NUM:
			return parseNumber();
		case STR:
			return parseString();
		case LCU:
			return parseObject();
		case LBR:
			return parseArray();
		case FAL:
			return parseFalse();
		case TRU:
			return parseTrue();
		case NUL:
			return parseNull();
		default:
			throw ParseError();
		}
	}
	std::shared_ptr<Node> parseArray()
	{
		std::shared_ptr<ArrayNode> ret = std::make_shared<ArrayNode>();
		scanner.match(LBR);
		if (scanner.lookahead() == RBR)
		{
			scanner.next();
			return ret;
		}
		ret->asVectorRef().push_back(parseValue());
		while (scanner.lookahead() == COM)
		{
			scanner.next();
			ret->asVectorRef().push_back(parseValue());
		}
		scanner.match(RBR);
		return ret;
	}
	std::shared_ptr<Node> parseObject()
	{
		std::shared_ptr<ObjectNode> ret = std::make_shared<ObjectNode>();
		scanner.match(LCU);
		if (scanner.lookahead() == RCU)
		{
			scanner.next();
			return ret;
		}
		std::string key;
		std::shared_ptr<Node> val;
		scanner.match(STR, key);
		scanner.match(COL);
		val = parseValue();
		ret->asMapRef().insert(std::make_pair(
			std::string(key.begin() + 1, key.end() - 1), val));
		while (scanner.lookahead() == COM)
		{
			scanner.next();
			scanner.match(STR, key);
			scanner.match(COL);
			val = parseValue();
			ret->asMapRef().insert(std::make_pair(
				std::string(key.begin() + 1, key.end() - 1), val));
		}
		scanner.match(RCU);
		return ret;
	}
private:
	Scanner scanner;
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

JSON JSON::fromNumber(double num) {
	std::stringstream ss;
	ss << num;
	auto n = std::make_shared<NumberNode>(ss.str());
	return JSON(n);
}

JSON JSON::fromBool(bool b)
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

JSON JSON::fromString(const std::string& str)
{
	return JSON(std::make_shared<StringNode>(str));
}

JSON JSON::makeObject()
{
	return JSON(std::make_shared<ObjectNode>());
}

JSON JSON::makeArray()
{
	return JSON(std::make_shared<ArrayNode>());
}

JSON::JSON(const char *content) {
	Parser p = Parser(Scanner(content));
	node = p.parseValue();
}

JSON JSON::field(const std::string& key) const
{
	return JSON(node->field(key));
}

JSON JSON::at(int idx) const
{
	return JSON(node->at(idx));
}

double JSON::asDouble() const
{
	return node->asDouble();
}

std::string JSON::asString() const
{
	std::stringstream ss;
	auto v = std::make_shared<PrintVisitor<std::stringstream>>(ss);
	node->acceptVisitor(v);
	return ss.str();
}

bool JSON::asBool() const
{
	return node->asBool();
}

JSON& JSON::append(const JSON& child)
{
	node->asVectorRef().push_back(child.node);
	return *this;
}

JSON& JSON::set(int idx, const JSON& child)
{
	node->asVectorRef()[idx] = child.node;
	return *this;
}

JSON& JSON::set(const std::string& field, const JSON& value)
{
	node->asMapRef()[field] = value.node;
	return *this;
}

std::vector<std::string> JSON::allFields() const
{
	auto& mp = node->asMapRef();
	std::vector<std::string> result;
	for (auto i = mp.begin(); i != mp.end(); ++i)
	{
		result.push_back(i->first);
	}
	return result;
}

int JSON::arraySize() {
	return node->asVectorRef().size();
}