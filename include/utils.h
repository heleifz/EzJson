#ifndef __EZ_JSON_UTILS__
#define __EZ_JSON_UTILS__

#include <cstdlib>
#include <cstring>

class AllocationError {};

struct ChunkInfo {
	ChunkInfo *next;
	size_t capacity;
	size_t used;
};

// TODO : ´íÎó´¦Àí
class MemoryPool
{
public:
	const static size_t CHUNK_SIZE = 128 * 1024;
	MemoryPool() : current(nullptr), firstChunk(nullptr)
	{
		newChunk(CHUNK_SIZE);
	}
	~MemoryPool()
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
			throw AllocationError();
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
// using memory pool
class String {
public:
	String() : begin(0), end(0) {}
	String(const char *b, const char *e) : begin(b), end(e) {}
	String(const char *b, const char *e, MemoryPool& pool)
	{
		int sz = e - b;
		void *buffer = pool.alloc(sz);
		memcpy(buffer, b, sz);
		begin = (const char*)buffer;
		end = begin + sz;
	}
	size_t size()
	{
		return end - begin;
	}
private:
	const char *begin;
	const char *end;
};

#endif