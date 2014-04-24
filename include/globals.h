#ifndef __EZ_JSON_GLOBALS__
#define __EZ_JSON_GLOBALS__

class ParseError {};
class ScanError {};
class NumberOverflowError {};

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

#endif