#ifndef __EZ_JSON__
#define __EZ_JSON__

bool isSymbol(const char*& str, char sym) {
	if (*str == sym) {
		str++;
		return true;
	}
	return false;
}

bool isZeroDigit(const char*& str) {
	return isSymbol(str, '0');
}

bool isNonZeroDigit(const char*& str) {
	const char table[] =
	{ '1', '2', '3', '4',
	'5', '6', '7', '8',
	'9' };

	for (int i = 0; i < 9; ++i) {
		if (isSymbol(str, table[i])) {
			return true;
		}
	}
	return false;
}

bool isDigit(const char*& str) {
	return isZeroDigit(str) || isNonZeroDigit(str);
}

template <typename Pred>
bool repetition(const char*& str, Pred pred) {
	while (pred(str));
	return true;
}

template <typename Pred>
bool oneOrMore(const char*& str, Pred pred) {
	if (!pred(str)) {
		return false;
	}
	return repetition(str, pred);
}

bool isNumber(const char*& str) {
	// optional minus sign
	isSymbol(str, '-');
	// zero, or more than zero digits (cannot start with 0)
	if (!((isZeroDigit(str) ||
		(isNonZeroDigit(str) && repetition(str, isDigit))))) {
		return false;
	}

	// real number
	if (isSymbol(str, '.')) {
		if (!oneOrMore(str, isDigit)) {
			return false;
		}
	}

	// scientific notation
	if (isSymbol(str, 'e') || isSymbol(str, 'E')) {
		// eat optional +/- sign
		(isSymbol(str, '+') || isSymbol(str, '-'));
		if (!oneOrMore(str, isDigit)) {
			return false;
		}
	}
	
	return true;
}

// ugly
bool isStringComponent(const char*& str) {
	const char table[] =
	{
		'"', '\\', '/', 'b', 'f', 'n', 'r', 't', 'u'
	};
	// look ahead
	if (*str == '"' || *str == '\0') {
		return false;
	}
	if (isSymbol(str, '\\')) {
		for (int i = 0; i < 8; ++i) {
			if (isSymbol(str, table[i])) {
				return true;
			}
		}
		if (isSymbol(str, table[8])) {
			if (!(isDigit(str) && isDigit(str) && isDigit(str) && isDigit(str))) {
				return false;
			}
			return true;
		}
		return false;
	} else {
		str++;
		return true;
	}
}

bool isString(const char*& str) {
	if (!isSymbol(str, '"')) {
		return false;
	}
	repetition(str, isStringComponent);
	if (!isSymbol(str, '"')) {
		return false;
	}
	return true;
}

bool isNull(const char*& str) {
	return (isSymbol(str, 'n') && isSymbol(str, 'u') &&
			isSymbol(str, 'l') && isSymbol(str, 'l'));
}

bool isTrue(const char*& str) {
	return (isSymbol(str, 't') && isSymbol(str, 'r') &&
			isSymbol(str, 'u') && isSymbol(str, 'e'));
}

bool isFalse(const char*& str) {
	return (isSymbol(str, 'f') && isSymbol(str, 'a') &&
			isSymbol(str, 'l') && isSymbol(str, 's') && isSymbol(str, 'e'));
}

bool isArray(const char*& str);
bool isObject(const char*& str);

bool isValue(const char*& str) {
	if (
		isNull(str) ||
		isFalse(str) ||
		isTrue(str) ||
		isNumber(str) ||
		isString(str) ||
		isObject(str) ||
		isArray(str)
		) {
		return true;
	}
	return false;
}

bool isArray(const char*& str) {
	if (!isSymbol(str, '[')) {
		return false;
	}
	// allow empty array
	if (isValue(str)) {
		repetition(str, [](const char*& s) {
			return isSymbol(s, ',') && isValue(s);
		});
	}
	if (!isSymbol(str, ']')) {
		return false;
	}
	return true;
}

bool isPair(const char*& str) {
	return isString(str) && isSymbol(str, ':') && isValue(str);
}

bool isObject(const char*& str) {
	if (!isSymbol(str, '{')) {
		return false;
	}
	// allow empty object
	if (isPair(str)) {
		repetition(str, [](const char*& s) {
			return isSymbol(s, ',') && isPair(s);
		});
	}
	if (!isSymbol(str, '}')) {
		return false;
	}
	return true;
}

#endif