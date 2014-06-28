#ifndef __EZ_JSON_PARSER__
#define __EZ_JSON_PARSER__

#include "globals.h"

namespace Ez
{

class DefaultAction
{
public:

	void stringAction(const char*, const char*) {}
	void numberAction(double) {}
	void boolAction(bool) {}
	void nullAction() {}
	void beginArrayAction() {}
	void endArrayAction(size_t) {}
	void beginObjectAction() {}
	void endObjectAction(size_t) { }
};

template <typename Scanner, typename Actions = DefaultAction>
class Parser : public INonCopyable
{
private:

	Scanner scanner;
	Actions& act;

public:

	Parser(const Scanner& sc, Actions& a)
		: scanner(sc), act(a)
	{}

	void parseNumber()
	{
		double val;
		scanner.matchDouble(val);
		act.numberAction(val);
	}

	void parseTrue()
	{
		scanner.match(TRU);
		act.boolAction(true);
	}

	void parseFalse()
	{
		scanner.match(FAL);
		act.boolAction(false);
	}

	void parseNull()
	{
		scanner.match(NUL);
		act.nullAction();
	}

	void parseString()
	{
		const char *b, *e;
		scanner.matchString(b, e);
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
		case NUL:
			parseNull();
			break;
		default:
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
};

} // namespace Ez

#endif