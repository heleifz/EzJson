#ifndef __EZ_JSON_GLOBALS__
#define __EZ_JSON_GLOBALS__

#include <utility>
#include <vector>
#include <string>

namespace Ez
{

/**
 * Token type
 */

enum TokenType
{
	EOS = 0,
	LCU = '{', RCU = '}', LBR = '[',
	RBR = ']', COM = ',', COL = ':',
	NUM, STR, TRU,
	FAL, NUL, CMT
};


/**
 * Interface
 */

class INonCopyable
{
protected:

	// Cannot create or delete directly (only through derived class)
	INonCopyable() {}
	~INonCopyable() {}

private:

	// Copying is not allowed
	INonCopyable(const INonCopyable&);
	INonCopyable& operator=(const INonCopyable&);
};

/**
 * Internal Exceptions
 */

class ParseError : public std::runtime_error
{
protected:
	/**
	 * @brief Get description of the token
	 * 
	 * @param t token
	 * @return token description
	 */
	const char *getTokenName(TokenType t) const
	{
		switch (t)
		{
		case EOS:
			return "End of string";
		case LCU:
			return "{";	
		case RCU:
			return "}";
		case LBR:
			return "[";
		case RBR:
			return "]";
		case COM:
			return ",";
		case COL:
			return ":";
		case NUM:
			return "Number";
		case STR:
			return "String";
		case TRU:
			return "true";
		case FAL:
			return "false";
		case NUL:
			return "null";
		case CMT:
			return "comment";
		default:
			return "unknown";
		}
	}

public:
	ParseError(const std::string& message) : std::runtime_error(message)
	{}
};

class UnexpectedTokenError : public ParseError
{
private:

	std::string formatMessage(TokenType expected, TokenType given) const
	{
		std::string message("Unexpected token : ");	
		message += getTokenName(given);
		message += ", expect ";
		message += getTokenName(expected);
		message += ".";
		return message;
	}

	std::string formatMessage(TokenType given) const
	{
		std::string message("Unexpected token : ");	
		message += getTokenName(given);
		message += ".";
		return message;
	}

public:

	UnexpectedTokenError(TokenType expected, TokenType given)
		: ParseError(formatMessage(expected, given))
	{
	}

	UnexpectedTokenError(TokenType given)
		: ParseError(formatMessage(given))
	{
	}

};

class IllegalCommentError : public ParseError
{
public:
	IllegalCommentError()
		: ParseError("Illegal comment format, expect '/' or '*' after first '/'")
	{
	}
};

class UnexpectedCharacterError : public ParseError
{
private:
	std::string formatMessage(char badChar, TokenType expected) const
	{
		std::string message("Unexpected character : ");	
		message += badChar;
		message += " , near token ";
		message += getTokenName(expected);
		message += ".";
		return message;
	}	

	std::string formatMessage(const std::string& badStr, TokenType expected) const
	{
		std::string message("Unexpected string : ");	
		message += badStr;
		message += ", expect ";
		message += getTokenName(expected);
		message += ".";
		return message;
	}	

public:
	UnexpectedCharacterError(const std::string& badStr, TokenType expected)
		: ParseError(formatMessage(badStr, expected))
	{
	}

	UnexpectedCharacterError(char ch, TokenType t)
		: ParseError(formatMessage(ch, t))
	{
	}
};

class InvalidCStringError : public std::exception
{
public:
	const char* what() const throw()
	{
		return "Invalid C-string pointer when initializing internal String object.";	
	}
};

class OutOfMemoryError : public std::exception
{
public:
	const char* what() const throw()
	{
		return "System running out of memory.";	
	}
};

class IndexOutOfRangeError : public std::exception
{
public:
	const char* what() const throw()
	{
		return "Accessing an invalid index.";
	}
};

class NumberOverflowError : public std::exception
{
public:
	const char* what() const throw()
	{
		return "Floating-point number is too large.";
	}
};

class NotAnArrayError : public std::exception
{
public:
	const char* what() const throw()
	{
		return "Current JSON node is not an array.";
	}	
};

class NotAnObjectError : public std::exception
{
public:
	const char* what() const throw()
	{
		return "Current JSON node is not an object.";
	}	
};

class NotAnArrayOrObjectError : public std::exception
{
public:
	const char* what() const throw()
	{
		return "Current JSON node is neither an array nor an object.";
	}	
};

class NotConvertibleError : public std::exception
{
public:
	const char* what() const throw()
	{
		return "Cannot convert current node value to expected type.";
	}	
};


} // namespace Ez

#endif