#ifndef __EZ_JSON__
#define __EZ_JSON__

#include <string>
#include <memory>
#include <vector>

enum TokenType
{
	EOS = 0,
	LCU = '{',
	RCU = '}',
	LBR = '[',
	RBR = ']',
	COM = ',',
	COL = ':',
	NUM,
	STR,
	TRU,
	FAL,
	NUL,
	CMT
};

class Node;

// fluent interface
class JSON {
public:
	static JSON fromNumber(double num);
	static JSON fromBool(bool b);
	static JSON fromString(const std::string& str);
	static JSON makeObject();
	static JSON makeArray();
	// construct from file
	explicit JSON(const char *path);
	JSON field(const std::string& key) const;
	JSON at(int idx) const;
	double asDouble() const;
	std::string asString() const;
	bool asBool() const;
	JSON& append(const JSON& child);
	JSON& set(int idx, const JSON& child);
	JSON& set(const std::string& field, const JSON& value);
	std::vector<std::string> allFields() const;
	int arraySize();
private:
	JSON(std::shared_ptr<Node> data) : node(data) {}
	std::shared_ptr<Node> node;
};

template <typename STREAM>
STREAM& operator<<(STREAM& s, const JSON& json)
{
	s << json.asString();
	return s;
}

#endif