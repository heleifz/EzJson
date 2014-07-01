#ifndef __EZ_JSON_ALLOCATOR__
#define __EZ_JSON_ALLOCATOR__

#include "globals.h"

#include <cstdlib>
#include <cstring>

namespace Ez
{

/**
 * @brief Custom allocator
 * @details A simple allocator with very high throughput
 * 
 */
class FastAllocator : public INonCopyable
{
private:

	/**
	 * @brief Page information that store at the begining of a page
	 * 
	 */
	struct PageInfo
	{
		// next page pointer
		PageInfo *next;
		size_t capacity;
		size_t used;
	};

	const static size_t PAGE_SIZE = 4 * 1024;

	PageInfo *current;
	PageInfo *firstPage;

public:

	/**
	 * @brief Initialize the allocator
	 * 
	 */
	FastAllocator() : current(nullptr), firstPage(nullptr)
	{
		newPage(PAGE_SIZE);
	}

	/**
	 * @brief Destroy the allocator 
	 */
	~FastAllocator()
	{
		clearAll();
	}

	/**
	 * @brief Allocate sz bytes from pool
	 * 
	 * @param sz size of required block
	 * @return address to the memory block
	 */
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

	/**
	 * @brief Expand existing memory block
	 * 
	 * @param old Pointer to old block
	 * @param old_sz size of old block
	 * @param new_sz size of new block
	 * @return Pointer to expanded block
	 */
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

	/**
	 * @brief Deallocate the memory pool
	 */
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

	/**
	 * @brief Require a new page from system
	 * 
	 * @param sz size of the page
	 */
	void newPage(size_t sz)
	{
		if (sz < PAGE_SIZE)
		{
			sz = PAGE_SIZE;
		}
		// store pageinfo at the head of the block
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