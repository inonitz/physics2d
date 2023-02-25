#pragma once
#include "base.hpp"




template<typename T, bool DontFreeCreationPointer = false> struct StaticPoolAllocator
{
private:
	using optionalOwnership = void*;
	struct Node {
		i64   index;
		Node* next;
	};

	
	Node* freelist; /* Book-keeping -> Each node points to a T object in memory. */
	Node* available;
	T*    buffer;   /* The actual memory allocated. */
	u64   elemCount;
	u64   freeBlk;


	bool   occupied(size_t idx) 		  { return freelist[idx].index < 0; 	   }
	size_t index_from_pointer(T const* p) { return __scast(size_t, (buffer - p) ); }

	void common_init(size_t amountOfElements)
	{
		ifcrashdbg(amountOfElements == 0);
		elemCount = amountOfElements; 
		freeBlk   = amountOfElements;
		for(size_t i = 0; i < amountOfElements - 1; ++i)
		{
			freelist[i].index = i + 1;
			freelist[i].next = &freelist[i + 1];
		}
		freelist[elemCount - 1] = { elemCount, nullptr }; /* last element shouldn't pointer anywhere */
		available = &freelist[0];
		return;
	}

public:
	void create(size_t amountOfElements)
	{
		buffer   = _mm_malloc(sizeof(T)    * amountOfElements, sizeof(T)   );
		freelist = _mm_malloc(sizeof(Node) * amountOfElements, sizeof(Node));
		common_init(amountOfElements);
		return;
	}


	/* 
		NOTE: 
			template boolean DontFreeCreationPointer is intended ONLY 
			for users who still need ownership of the underlying memory being managed.
			I won't advise creating the structure with DontFreeCreationPointer=TRUE, unless you're
			CERTAIN that you'll supply a memory buffer to the allocator. (unless you like memory leaks >:) )


		proper_memory = buffer_aligned_to_sizeof_T;
		if(DontFreeCreationPointer) {
			Memory will only be managed internally, freeing the Buffer is on the user.
		} else {
			proper_memory = proper_memory && buffer_came_from_mm_malloc!!!
			In this case the buffer will be completely managed by the allocator.
		} 
	*/
	void create(optionalOwnership* __proper_memory, size_t amountOfElements)
	{
		buffer   = __scast(T*, __proper_memory);
		freelist = _mm_malloc(sizeof(Node) * amountOfElements, sizeof(Node));
		common_init(amountOfElements);
		return;
	}


	void destroy() 
	{
		if(!DontFreeCreationPointer) {
			_mm_free(buffer);
		}
		
		_mm_free(freelist);
		available = nullptr;
		elemCount = 0;
		freeBlk   = 0;
		return;
	}


	T* allocate() 
	{
		ifcrashdbg(!freeBlk);
		T* v = &buffer[available->index - 1];
		available->index *= -1; /* now occupied */

		available = available->next;
		--freeBlk;
		return v;
	}


	void free(T* ptr)
	{
		size_t idx = index_from_pointer(ptr);
		ifcrashdbg(!isaligned(ptr, sizeof(T)) || occupied(idx) || (freeBlk == elemCount));
		freelist[idx].index *= -1;
		freelist[idx].next = available;
		available = &freelist[idx];
		++freeBlk;
		return;
	}


	size_t availableBlocks() const { return freeBlk;   }
	size_t size()    	     const { return elemCount; }

	
	void print()
	{
		static const char* strs[2] = { "Occupied", "Free    " };
		bool tmp = false;
		printf("Static Pool Allocator:\nObject Array[%llu]: %p\n    Free:     %u\n    Occupied: %u\n    ", buffer, elemCount, freeBlk, elemCount - freeBlk);
		for(size_t i = 0; i < elemCount; ++i)
		{
			tmp = boolean(freelist[i].index > 0);
			printf("    Object [i = %llu] [%s] => Object [%llu]\n", i, strs[tmp], __scast(u64, freelist[i].index));
		}
		return;
	}
};