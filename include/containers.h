#ifndef __EZ_JSON_UTILS__
#define __EZ_JSON_UTILS__

#include <cstdlib>
#include <cstring>

struct ChunkInfo
{
	ChunkInfo *next;
	size_t capacity;
	size_t used;
};

template <typename U, typename V>
struct Pair
{
	U key;
	V val;
};

// todo realloc
class ChunkAllocator
{
public:
	const static size_t CHUNK_SIZE = 128 * 1024;
	ChunkAllocator() : current(nullptr), firstChunk(nullptr)
	{
		newChunk(CHUNK_SIZE);
	}
	~ChunkAllocator()
	{
		clearAll();
	}
	void* alloc(size_t sz)
	{
		if (current->used + sz > current->capacity)
		{
			newChunk(sz + sizeof(ChunkInfo));
		}
		void* ret = ((char*)current) + current->used;
		current->used += sz;
		return ret;
	}
	void* reAlloc(void *old, size_t old_sz, size_t new_sz)
	{
		if (new_sz <= old_sz)
		{
			return old;
		}
		else if (old == ((char*)current) + current->used - old_sz)
		{
			size_t diff = new_sz - old_sz;
			if (current->used + diff <= current->capacity)
			{
				current->used += diff;
				return old;
			}
		}
		void* ret = alloc(new_sz);
		memcpy(ret, old, old_sz);
		return ret;
	}
private:
	void clearAll()
	{
		for (ChunkInfo *f = firstChunk; f != nullptr;)
		{
			ChunkInfo *next = f->next;
			free(f);
			f = next;
		}
		firstChunk = nullptr;
		current = nullptr;
	}
	void newChunk(size_t sz)
	{
		if (sz < CHUNK_SIZE)
		{
			sz = CHUNK_SIZE;
		}
		ChunkInfo *ret = (ChunkInfo*)malloc(sz);
		if (ret == nullptr)
		{
			throw OutOfMemoryError();
		}
		ret->capacity = sz;
		ret->used = sizeof(ChunkInfo);
		ret->next = nullptr;
		if (current == nullptr)
		{
			firstChunk = ret;
		}
		else
		{
			current->next = ret;
		}
		current = ret;
	}
	ChunkInfo *current;
	ChunkInfo *firstChunk;
};

// immutable string class
// using chunk allocator
// not a C-style string
class String {
public:
	String(const char *b)
	{
		size_t len = strlen(b);
		beginPtr = b;
		endPtr = b + len;
	}
	String(const char *b, const char *e) : beginPtr(b), endPtr(e)
	{}
	String(const char *b, ChunkAllocator& allocator)
	{
		int sz = strlen(b);
		void *buffer = allocator.alloc(sz);
		memcpy(buffer, b, sz);
		beginPtr = (const char*)buffer;
		endPtr = beginPtr + sz;
	}
	String(const char *b, const char *e, ChunkAllocator& allocator)
	{
		int sz = e - b;
		void *buffer = allocator.alloc(sz);
		memcpy(buffer, b, sz);
		beginPtr = (const char*)buffer;
		endPtr = beginPtr + sz;
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
	bool operator<(const String& other) const
	{
		size_t sz1 = size();
		size_t sz2 = size();
		size_t smaller = sz1 < sz2 ? sz1 : sz2;
		for (size_t i = 0; i < smaller; ++i)
		{
			if (beginPtr[i] < other.beginPtr[i])
			{
				return true;
			}
			else if (beginPtr[i] > other.beginPtr[i])
			{
				return false;
			}
		}
		if (sz1 >= sz2)
		{
			return false;
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
	// get const iterator of the string
	const char* const begin() const
	{
		return beginPtr;
	}
	const char* const end() const
	{
		return endPtr;
	}
private:
	const char *beginPtr;
	const char *endPtr;
};

template <typename T>
class Array
{
public:
	const static size_t INIT_CAPACITY = 10;
	Array(ChunkAllocator& allocator, size_t ca = INIT_CAPACITY)
		: capacity(ca), sz(0)
	{
		data = static_cast<T*>(allocator.alloc(capacity * sizeof(T)));
	}
	size_t size() const
	{
		return sz;
	}
	void pushBack(const T& e, ChunkAllocator& allocator)
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
	const T& operator[](size_t idx) const
	{
		if (idx >= sz)
		{
			throw IndexOutOfRangeError();
		}
		return data[idx];
	}
	T& operator[](size_t idx)
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
private:
	// prevent copy
	Array(const Array<T>&) {}
	Array<T>& operator=(const Array<T>&) {}

	size_t capacity;
	size_t sz;
	T *data;
};

// dictionary class (using array)
template <typename T>
class Dictionary
{
public:
	Dictionary(ChunkAllocator& allocator) 
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
		return data[result].val;
	}
	void insert(const String& k, const T& v, ChunkAllocator& allocator)
	{
		data.pushBack({ k, v }, allocator);
	}
	void set(const String& k, const T& v)
	{
		int result = find(k);
		if (result == -1)
		{
			throw IndexOutOfRangeError();
		}
		data[result].val = v;
	}
	// const iterators
	const T* const begin()
	{
		return data.begin();
	}
	const T* const end()
	{
		return data.end();
	}
private:
	// prevent copy
	Dictionary(const Dictionary<T>&) {}
	Dictionary<T>& operator=(const Dictionary<T>&) {}

	int find(const String& key) const
	{
		int result = 0;
		for (; result < static_cast<int>(data.size()); ++result)
		{
			if (data[result].key == key)
			{
				return result;
			}
		}
		return -1;
	}

	Array<Pair<String, T> > data;
};

#endif