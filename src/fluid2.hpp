#pragma once
#include "vec.hpp"
#include <functional>




template<typename T> struct StaticPoolAllocator
{
private:
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

public:
	void create(size_t amountOfElements)
	{
		elemCount = amountOfElements; 
		freeBlk   = amountOfElements;
		buffer   = _mm_malloc(sizeof(T)    * amountOfElements, sizeof(T)   );
		freelist = _mm_malloc(sizeof(Node) * amountOfElements, sizeof(Node));

		for(size_t i = 0; i < amountOfElements - 1; ++i)
		{
			freelist[i].index = i + 1;
			freelist[i].next = &freelist[i + 1];
		}
		freelist[elemCount - 1] = { elemCount, nullptr }; /* last element shouldn't pointer anywhere */
		available = &freelist[0];
		return;
	}


	void destroy() 
	{
		/* I could memset(0) the whole struct after de-allocating the memory but nah */
		_mm_free(buffer);
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


__force_inline u64 MurmurHash64A(const void * key, i32 len, u64 seed)
{
	const u64 m = 0xc6a4a7935bd1e995LLU;
	const int r = 47;

	u64 h = seed ^ (len * m), k = 0;

	const u64 * data = (const u64 *)key;
	const u64 * end = (len >> 3) + data;

	while(data != end)
	{
		k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 

		h ^= k;
		h *= m; 
	}

	const u8* data2 = (const u8*)data;
	switch(len & 7)
	{
	case 7: h ^= (u64)(data2[6]) << 48;
	case 6: h ^= (u64)(data2[5]) << 40;
	case 5: h ^= (u64)(data2[4]) << 32;
	case 4: h ^= (u64)(data2[3]) << 24;
	case 3: h ^= (u64)(data2[2]) << 16;
	case 2: h ^= (u64)(data2[1]) << 8;
	case 1: h ^= (u64)(data2[0]);
			h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
} 




struct GridCell {
	u8 type; /* Air, Fluid, Solid. */
};


/* 
	Hash table with Separate Chaining, 
	will try to use as many buckets as possible 
*/
template<typename Key, typename Value> struct Hashtable 
{
	using Allocator    = StaticPoolAllocator<GridCell>;
	using HashFunction = std::function<Value(Key)>;

	Allocator    underlying_manager;
	HashFunction hash;
	

	void create(u32 size);
	void destroy();

	Value lookup(ref<Key> k);
	bool  del(ref<Key> k);
	bool  insert(ref<Value> v, ref<Key> k);


};




struct Fluid
{
	f32 dx;
	f32 u_max;
	f32 k_cfl;
	f32 dt;	
};