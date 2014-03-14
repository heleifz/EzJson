#ifndef __EZ_JSON__
#define __EZ_JSON__

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cctype>

class NumberNode;
class StringNode;
class BoolNode;
class ArrayNode;
class ObjectNode;

class Visitor : public std::enable_shared_from_this<Visitor> {
public:
	virtual void visit(NumberNode* ptr) = 0;
	virtual void visit(BoolNode* ptr) = 0;
	virtual void visit(StringNode* ptr) = 0;
	virtual void visit(ArrayNode* ptr) = 0;
	virtual void visit(ObjectNode* ptr) = 0;
};

// abstract syntax tree

class Node {
public:
	virtual ~Node() { }
	virtual void acceptVisitor(std::shared_ptr<Visitor> visitor) = 0;
};

class NumberNode : public Node {
public:
	void acceptVisitor(std::shared_ptr<Visitor> visitor) {
		visitor->visit(this);
	}
	NumberNode(const std::string& val) {
		value = stod(val);
	}
	double value;
};

// string
class StringNode : public Node {
public:
	StringNode(const std::string& val) : value(val) { }
	void acceptVisitor(std::shared_ptr<Visitor> visitor) {
		visitor->visit(this);
	}
	std::string value;
};

// true / false
class BoolNode : public Node {
public:
	void acceptVisitor(std::shared_ptr<Visitor> visitor) {
		visitor->visit(this);
	}
	BoolNode(const std::string& val) {
		if (val == "true") {
			value = true;
		}
		else {
			value = false;
		}
	}
	bool value;
};

// do nothing, store nothing
// null node is "nullptr"

class ArrayNode : public Node {
public:
	void acceptVisitor(std::shared_ptr<Visitor> visitor) {
		visitor->visit(this);
	}
	std::vector<std::shared_ptr<Node>> childs;
};

class ObjectNode : public Node {
public:
	void acceptVisitor(std::shared_ptr<Visitor> visitor) {
		visitor->visit(this);
	}
	std::map<std::string, std::shared_ptr<Node>> pairs;
};

//////////////////////////////////////////////////////////////////

// the LL(1) json parser

class Parser {
private:
	// primitive "eaters", they dont generate result
	static bool eatSymbol(const char*& str, char sym) {
		if (*str == sym) {
			str++;
			return true;
		}
		return false;
	}

	static bool eatZeroDigit(const char*& str) {
		return eatSymbol(str, '0');
	}

	static bool eatNonZeroDigit(const char*& str) {
		const char table[] =
		{ '1', '2', '3', '4',
		'5', '6', '7', '8',
		'9' };
		for (int i = 0; i < 9; ++i) {
			if (eatSymbol(str, table[i])) {
				return true;
			}
		}
		return false;
	}

	static bool eatDigit(const char*& str) {
		return eatZeroDigit(str) || eatNonZeroDigit(str);
	}

	// number / string / true / false / null  -> return a "result" string

	static bool eatNumber(const char*& str, std::string& result) {
		const char* start = str;

		// optional minus sign
		eatSymbol(str, '-');

		// zero, or more than zero digits (cannot start with 0)
		if (!eatZeroDigit(str)) {
			if (!eatNonZeroDigit(str)) {
				return false;
			}
			while (eatDigit(str));
		}

		// real number
		if (eatSymbol(str, '.')) {
			// at least one
			if (!eatDigit(str)) {
				return false;
			}
			// eat rest
			while (eatDigit(str));
		}

		// scientific notation
		if (eatSymbol(str, 'e') || eatSymbol(str, 'E')) {
			// eat optional +/- sign
			(eatSymbol(str, '+') || eatSymbol(str, '-'));
			if (!eatDigit(str)) {
				return false;
			}
			// eat rest
			while (eatDigit(str));
		}

		result = std::string(start, str);

		return true;
	}

	// eat string

	static bool eatString(const char*& str, std::string& result) {
		const char* start = str;

		const char table[] =
		{
			'"', '\\', '/', 'b', 'f', 'n', 'r', 't', 'u'
		};
		if (!eatSymbol(str, '"')) {
			return false;
		}
		// look ahead
		while (*str != '"') {
			// end of str, format error.
			if (*str == '\0') {
				return false;
			}
			else if (eatSymbol(str, '\\')) {
				int i = 0;
				for (; i < 8; ++i) {
					if (eatSymbol(str, table[i])) {
						break;
					}
				}
				// special char found
				if (i < 8) {
					continue;
				}
				else if (eatSymbol(str, table[8])) {
					if (!(eatDigit(str) && eatDigit(str) && eatDigit(str) && eatDigit(str))) {
						return false;
					}
					continue;
				}
				else {
					return false;
				}
			}
			else {
				// eat anything else
				str++;
			}
		}
		if (!eatSymbol(str, '"')) {
			return false;
		}

		result = std::string(start, str);

		return true;
	}

	// eat null

	static bool eatNull(const char*& str, std::string& result) {
		if (eatSymbol(str, 'n') && eatSymbol(str, 'u') &&
			eatSymbol(str, 'l') && eatSymbol(str, 'l')) {
			result = "null";
			return true;
		}
		return false;
	}

	// eat true / false

	static bool eatTrue(const char*& str, std::string& result) {
		if (eatSymbol(str, 't') && eatSymbol(str, 'r') &&
			eatSymbol(str, 'u') && eatSymbol(str, 'e')) {
			result = "true";
			return true;
		}
		return false;
	}

	static bool eatFalse(const char*& str, std::string& result) {
		if (eatSymbol(str, 'f') && eatSymbol(str, 'a') &&
			eatSymbol(str, 'l') && eatSymbol(str, 's') && eatSymbol(str, 'e')) {
			result = "false";
			return true;
		}
		return false;
	}

	// compound value : returns a tree

	static bool eatValue(const char*& str, std::shared_ptr<Node>& result) {
		std::string val;
		if (eatNull(str, val)) {
			result = std::shared_ptr<Node>(nullptr);
			return true;
		}
		else if (eatFalse(str, val) || eatTrue(str, val)) {
			result = std::make_shared<BoolNode>(val);
			return true;
		}
		else if (eatNumber(str, val)) {
			result = std::make_shared<NumberNode>(val);
			return true;
		}
		else if (eatString(str, val)) {
			result = std::make_shared<StringNode>(val);
			return true;
		}
		else if (eatArray(str, result)) {
			return true;
		}
		else if (eatObject(str, result)) {
			return true;
		}

		return false;
	}

	static bool eatArray(const char*& str, std::shared_ptr<Node>& result) {
		std::shared_ptr<ArrayNode> ret = std::make_shared<ArrayNode>();

		if (!eatSymbol(str, '[')) {
			return false;
		}
		// allow empty array
		if (*str != ']') {
			std::shared_ptr<Node> one;
			// value ....
			if (!eatValue(str, one)) {
				return false;
			}
			ret->childs.push_back(one);

			// ,value  ,value (trailling comma is not allowed)
			while (eatSymbol(str, ',')) {
				if (!eatValue(str, one)) {
					return false;
				}
				ret->childs.push_back(one);
			}
		}
		if (!eatSymbol(str, ']')) {
			return false;
		}

		result = ret;

		return true;
	}

	static bool eatObject(const char*& str, std::shared_ptr<Node>& result) {
		std::shared_ptr<ObjectNode> ret = std::make_shared<ObjectNode>();

		if (!eatSymbol(str, '{')) {
			return false;
		}
		// allow empty object
		// look ahead
		if (*str != '}') {
			std::string key;
			std::shared_ptr<Node> val;

			if (!(eatString(str, key) && eatSymbol(str, ':') && eatValue(str, val))) {
				return false;
			}
			ret->pairs.insert(std::make_pair(key, val));
			// , str : val  , str : val ...
			while (eatSymbol(str, ',')) {
				if (!(eatString(str, key) && eatSymbol(str, ':') && eatValue(str, val))) {
					return false;
				}
				ret->pairs.insert(std::make_pair(key, val));
			}
		}
		if (!eatSymbol(str, '}')) {
			return false;
		}
		result = ret;
		return true;
	}
public:
	static std::shared_ptr<Node> parse(const std::string& str) {
		std::string trimmed = str;

		// remove comment line
		// todo...

		// remove white space charactor
		trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), [](const char& c) {
			return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
		}), trimmed.end());

		std::shared_ptr<Node> ret;
		const char* ptr = trimmed.c_str();
		if (eatObject(ptr, ret)) {
			return ret;
		}
		else {
			return nullptr;
		}
	}
};

#endif