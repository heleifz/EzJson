#ifndef __EZ_JSON_UTILS__
#define __EZ_JSON_UTILS__

#include "globals.h"

namespace Ez
{

/**
 * Immutable and copyable string class
 * Do not manage memory itself
 */
class String
{
private:

	const char *beginPtr;
	const char *endPtr;

public:

	String(const char *b)
	{
		size_t len = strlen(b);
		beginPtr = b;
		endPtr = b + len;
	}

	template <typename ALLOCATOR>
	String(const char *b, const char *e, ALLOCATOR& allocator)
	{
		size_t sz = e - b;
		if (sz < 0)
		{
			throw InvalidCStringError();
		}
		else if (sz > 0)
		{
			void *buffer = allocator.alloc(sz);
			memcpy(buffer, b, sz);
			beginPtr = (const char*)buffer;
			endPtr = beginPtr + sz;
		}
		else
		{
			beginPtr = nullptr;
			endPtr = nullptr;
		}
	}

	size_t size() const
	{
		return endPtr - beginPtr;
	}

	bool operator==(const String& other) const
	{
		size_t sz = size();
		if (sz != other.size())
		{
			return false;
		}
		for (size_t i = 0; i < sz; i++)
		{
			if (this->beginPtr[i] != other.beginPtr[i])
			{
				return false;
			}
		}
		return true;
	}

	char operator[](size_t idx) const
	{
		if (idx >= size())
		{
			throw IndexOutOfRangeError();
		}
		return beginPtr[idx];
	}

	const char* begin() const
	{
		return beginPtr;
	}

	const char* end() const
	{
		return endPtr;
	}

	std::string asSTLString() const
	{
		if (beginPtr == nullptr || endPtr == nullptr)
		{
			return "";
		}
		return std::string(beginPtr, endPtr);
	}
};

/**
 * Dynamic array that use allocator to manage memory,
 */

template <typename T, typename ALLOCATOR>
class Array : public INonCopyable
{
private:

	ALLOCATOR& allocator;
	const static size_t INIT_CAPACITY = 10;
	size_t capacity;
	size_t sz;
	T *data;

public:

	Array(ALLOCATOR& a, size_t ca = INIT_CAPACITY)
		: capacity(ca), sz(0), allocator(a)
	{
		data = static_cast<T*>(allocator.alloc(capacity * sizeof(T)));
	}

	void pushBack(const T& e)
	{
		if (sz == capacity)
		{
			void *newData = allocator.reAlloc(data, capacity * sizeof(T),
				capacity * 2 * sizeof(T));
			capacity *= 2;
			data = static_cast<T*>(newData);
		}
		data[sz++] = e;
	}

	T popBack()
	{
		if (sz == 0)
		{
			throw IndexOutOfRangeError();
		}
		return data[--sz];
	}

	void shrink(size_t amount)
	{
		if (amount > sz)
		{
			throw IndexOutOfRangeError();
		}
		sz -= amount;
	}

	void remove(size_t idx)
	{
		if (idx >= sz)
		{
			throw IndexOutOfRangeError();
		}
		for (size_t i = idx + 1; i < sz; ++i)
		{
			data[i - 1] = data[i];
		}
		sz--;
	}

	T& operator[](size_t idx)
	{
		if (idx >= sz)
		{
			throw IndexOutOfRangeError();
		}
		return data[idx];
	}

	size_t size() const
	{
		return sz;
	}

	const T& operator[](size_t idx) const
	{
		if (idx >= sz)
		{
			throw IndexOutOfRangeError();
		}
		return data[idx];
	}

	const T* const begin() const
	{
		return data;
	}

	const T* const end() const
	{
		return data + sz;
	}
};

/**
 * Naive dictionary implementation backed by a dynamic array,
 */
template <typename T, typename ALLOCATOR>

class Dictionary : public INonCopyable
{
private:

	Array<std::pair<String, T>, ALLOCATOR> data;

public:

	Dictionary(ALLOCATOR& allocator)
		: data(allocator)
	{
	}

	size_t size() const
	{
		return data.size();
	}

	bool contains(const String& k) const
	{
		return find(k) != -1;
	}

	const T& get(const String& k) const
	{
		int result = find(k);
		if (result == -1)
		{
			throw IndexOutOfRangeError();
		}
		return data[result].second;
	}

	std::vector<std::string> keys() const
	{
		std::vector<std::string> result;
		for (auto i = data.begin(); i != data.end(); ++i)
		{
			result.push_back(i->first.asSTLString());
		}
		return result;
	}

	void set(const String& k, const T& v)
	{
		int result = find(k);
		if (result == -1)
		{
			data.pushBack(std::make_pair(k, v));
		}
		else
		{
			data[result].second = v;
		}
	}

	void remove(const String& k)
	{
		int result = find(k);
		if (result == -1)
		{
			throw IndexOutOfRangeError();
		}
		else
		{
			data.remove(result);
		}
	}

	const std::pair<String, T>* const begin() const
	{
		return data.begin();
	}

	const std::pair<String, T>* const end() const
	{
		return data.end();
	}

private:

	int find(const String& key) const
	{
		int result = 0;
		int sz = static_cast<int>(data.size());
		for (; result < sz; ++result)
		{
			if (data[result].first == key)
			{
				return result;
			}
		}
		return -1;
	}
};

} // namespace Ez

#endif