#ifndef __EZ_JSON_GLOBALS__
#define __EZ_JSON_GLOBALS__

class ParseError {};
class ScanError {};

class OutOfMemoryError {};
class IndexOutOfRangeError {};
class DuplicateKeyError {};
class NumberOverflowError {};

class NotAnArrayError {};
class NotAnObjectError {};
class NotAnArrayOrObjectError {};
class NotConvertibleError {};

enum TokenType
{
	EOS = 0,
	LCU = '{', RCU = '}', LBR = '[',
	RBR = ']', COM = ',', COL = ':',
	NUM, STR, TRU,
	FAL, NUL, CMT
};

#endif