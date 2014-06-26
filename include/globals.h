#ifndef __EZ_JSON_GLOBALS__
#define __EZ_JSON_GLOBALS__

/**
 * Exceptions
 */

class ParseError {};
class ScanError {};

class InvalidArgumentError {};
class OutOfMemoryError {};
class IndexOutOfRangeError {};
class DuplicateKeyError {};
class NumberOverflowError {};
class EmptyStringError {};

class NotAnArrayError {};
class NotAnObjectError {};
class NotAnArrayOrObjectError {};
class NotConvertibleError {};

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

#endif