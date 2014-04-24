#ifndef __EZ_JSON_PARSER__
#define __EZ_JSON_PARSER__

#include <memory>
#include <iostream>

#include "scanner.h"
#include "utils.h"

class DefaultAction
{
public:
	int count;
	DefaultAction() : count(0) {}
	void stringAction(const char *b, const char *e) { count++; }
	void numberAction(double val) {}
	void trueAction() {}
	void falseAction() {}
	void nullAction() {}
	void beginArrayAction() {}
	void endArrayAction(size_t size) {}
	void beginObjectAction() {}
	void endObjectAction(size_t size) { }
};

template <typename Actions=DefaultAction>
class Parser
{
public:
	Parser(const Scanner& sc, Actions& a)
		: scanner(sc), act(a)
	{}
	void parseNumber()
	{
		double val;
		scanner.match(NUM, val);
		act.numberAction(val);
	}
	void parseTrue()
	{
		scanner.match(TRU);
		act.trueAction();
	}
	void parseFalse()
	{
		scanner.match(FAL);
		act.falseAction();
	}
	void parseNull()
	{
		scanner.match(NUL);
		act.nullAction();
	}
	void parseString()
	{
		const char *b, *e;
		scanner.match(STR, b, e);
		act.stringAction(++b, --e);
	}
	void parseValue()
	{
		switch (scanner.lookahead())
		{
		case NUM:
			parseNumber();
			break;
		case STR:
			parseString();
			break;
		case LCU:
			parseObject();
			break;
		case LBR:
			parseArray();
			break;
		case FAL:
			parseFalse();
			break;
		case TRU:
			parseTrue();
			break;
		default:
			parseNull();
			break;
		}
	}
	void parseArray()
	{
		size_t sz = 0;
		act.beginArrayAction();
		scanner.match(LBR);
		if (scanner.lookahead() == RBR)
		{
			scanner.next();
			act.endArrayAction(sz);
			return;
		}
		parseValue();
		sz++;
		while (scanner.lookahead() == COM)
		{
			scanner.next();
			parseValue();
			sz++;
		}
		act.endArrayAction(sz);
		scanner.match(RBR);
	}
	void parseObject()
	{
		size_t sz = 0;
		act.beginObjectAction();
		scanner.match(LCU);
		if (scanner.lookahead() == RCU)
		{
			scanner.next();
			act.endObjectAction(sz);
			return;
		}
		parseString();
		scanner.match(COL);
		parseValue();
		sz++;

		while (scanner.lookahead() == COM)
		{
			scanner.next();
			parseString();
			scanner.match(COL);
			parseValue();
			sz++;
		}
		act.endObjectAction(sz);
		scanner.match(RCU);
	}
private:
	Scanner scanner;
	Actions& act;
};

#endif