#ifndef __EZ_JSON_ALLOCATOR__
#define __EZ_JSON_ALLOCATOR__

#include "globals.h"

#include <cstdlib>
#include <cstring>

namespace Ez
{

class FastAllocator : public INonCopyable
{
private:

	struct PageInfo
	{
		PageInfo *next;
		size_t capacity;
		size_t used;
	};

	const static size_t PAGE_SIZE = 4 * 1024;
	PageInfo *current;
	PageInfo *firstPage;

public:

	FastAllocator() : current(nullptr), firstPage(nullptr)
	{
		newPage(PAGE_SIZE);
	}

	~FastAllocator()
	{
		clearAll();
	}

	void* alloc(size_t sz)
	{
		if (current->used + sz > current->capacity)
		{
			newPage(sz + sizeof(PageInfo));
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
		for (PageInfo *f = firstPage; f != nullptr;)
		{
			PageInfo *next = f->next;
			free(f);
			f = next;
		}
		firstPage = nullptr;
		current = nullptr;
	}

	void newPage(size_t sz)
	{
		if (sz < PAGE_SIZE)
		{
			sz = PAGE_SIZE;
		}
		PageInfo *ret = (PageInfo*)malloc(sz);
		if (ret == nullptr)
		{
			throw OutOfMemoryError();
		}
		ret->capacity = sz;
		ret->used = sizeof(PageInfo);
		ret->next = nullptr;
		if (current == nullptr)
		{
			firstPage = ret;
		}
		else
		{
			current->next = ret;
		}
		current = ret;
	}
};

} // namespace Ez

#endif