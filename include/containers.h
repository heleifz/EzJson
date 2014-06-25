#ifndef __EZ_JSON_UTILS__
#define __EZ_JSON_UTILS__

#include "globals.h"

#include <utility>
#include <vector>
#include <string>

/**
 * Immutable and copyable string class
 * Do not manage memory itself
 */
class String
{
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
			throw InvalidArgumentError();
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
		for (size_t i = 0; i < sz; ++i)
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

private:
	const char *beginPtr;
	const char *endPtr;
};

/**
 * Dynamic array that use allocator to manage memory, 
 * To improve efficiency, it is not copyable, but movable
 */
template <typename T>
class Array : public INonCopyable
{
private:

	const static size_t INIT_CAPACITY = 10;
	size_t capacity;
	size_t sz;
	T *data;

public:

	template <typename ALLOCATOR>
	Array(ALLOCATOR& allocator, size_t ca = INIT_CAPACITY)
		: capacity(ca), sz(0)
	{
		data = static_cast<T*>(allocator.alloc(capacity * sizeof(T)));
	}

	// not copyable, but movable
	Array(Array<T>&& other)
	{
		this->swap(std::move(other));
	}

	void move(Array<T>&& other)
	{
		std::swap(capacity, other.capacity);
		std::swap(sz, other.sz);
		std::swap(data, other.data);
	}
	
	template <typename ALLOCATOR>
	void pushBack(const T& e, ALLOCATOR& allocator)
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

	// const iterator
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
* To improve efficiency, it is not copyable, but movable
*/
template <typename T>

class Dictionary : public INonCopyable
{
private:
	Array<std::pair<String, T> > data;

public:
	template <typename ALLOCATOR>
	Dictionary(ALLOCATOR& allocator) 
		: data(allocator)
	{
	}

	// not copyable, but movable
	Dictionary(Dictionary<T>&& other)
		: data(std::move(other.data))
	{
	}

	void move(Dictionary<T>&& other)
	{
		data.move(std::move(other));
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

	template <typename ALLOCATOR>
	void insert(const String& k, const T& v, ALLOCATOR& allocator)
	{
		data.pushBack(std::make_pair(k, v), allocator);
	}

	void set(const String& k, const T& v)
	{
		int result = find(k);
		if (result == -1)
		{
			throw IndexOutOfRangeError();
		}
		data[result].second = v;
	}

	// const iterators
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
		for (; result < static_cast<int>(data.size()); ++result)
		{
			if (data[result].first == key)
			{
				return result;
			}
		}
		return -1;
	}
};

#endif