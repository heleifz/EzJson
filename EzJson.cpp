#include <string>

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

class ScanError {};
class ParseError {};

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
					type = CMT;
					return;
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
					type = CMT;
					return;
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
	bool match(TokenType t)
	{
		if (t == type)
		{
			next();
			return true;
		}
		return false;
	}
	bool match(TokenType t, std::string& out)
	{
		if (t == type)
		{
			out = std::string(tokenBegin, tokenEnd);
			next();
			return true;
		}
		return false;
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

// TODO : Parser