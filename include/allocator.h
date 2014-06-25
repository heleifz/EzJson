#ifndef __EZ_JSON_ALLOCATOR__
#define __EZ_JSON_ALLOCATOR__

#include "globals.h"

#include <cstdlib>
#include <cstring>

class ChunkAllocator : public INonCopyable
{
private:
	struct ChunkInfo
	{
		ChunkInfo *next;
		size_t capacity;
		size_t used;
	};

	const static size_t CHUNK_SIZE = 4 * 1024;
	ChunkInfo *current;
	ChunkInfo *firstChunk;

public:
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
};

#endif