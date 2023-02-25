#pragma once
#include "allocator.hpp"
#include <functional>
#include <vector>




/* 
	Hash table with Separate Chaining, 
	will try to use as many buckets as possible 

    Note: When objects are small, they should be coupled with their nodes into 1 
    structure, meaning Node {
        Key,
        Value
    }[]

    instead of Node {
        Key,
        Value*
    }[]
    
    The common factor in this is that nodes are allocated in tightly-packed arrays for each bucket.
    The difference in this case would be that values are allocated in a different array, unrelated to the 
    Node Array.
    

*/
template<typename Key, typename Value> struct Hashtable 
{
public:
    using HashFunction    = std::function<u64(Key)>;
    using CompareFunction = std::function<bool(Key, Key)>; /* MUST return 1 if keys match! */

private:
    static constexpr f32 m_growth_factor = 2.0f;
    static constexpr f32 m_load_factor   = 0.70f;
    /* 
        Nodes allocated are close in memory, 
        so this shouldn't be such a big problem. 
        still, we want to minimize the length of each linked list. 
        Need more time to think about this parameter and its usefulness.
    */
    static constexpr f32 m_max_nodes_per_bucket = 10.0f;

    /* 
        NOTE:
        Problem: Where 2 DIFFERENT Keys are mapped to the same hash (REALLY improbable).
        Partial Solution: add a unique ID to each node as an internal member.
        Moreover: Catching an error like this may only work by printing the status of the current hash-table to a file,
        along with the current element that is trying to be inserted.
    */
    struct Node {
        Key    k;
        Value* v;
        Node*  next;
    };
    using PNode = Node;


    struct Bucket {
        PNode* list;
        PNode* last;
        size_t length;

        PNode* begin() const { return list; }
        PNode* end()   const { return last; }
        void   append(PNode* already_allocated_and_init); /* add node to end of list */
    };


    using NodeManager  = StaticPoolAllocator<PNode>;
    using ValueManager = StaticPoolAllocator<Value>;

    NodeManager  nodes; 
    ValueManager values;
    std::vector<Bucket> buckets; /* Hashtable Capacity = maxAmountOfNodes; Hashtable Size = sum(buckets[i].size) */
	f32                 tableSize;

    HashFunction    hash_function;
    CompareFunction compare_function;



    __force_inline f32 loadFactor() const { return tableSize * ( 1 / __scast(f32, nodes.size()) ); }


    /* 
        Iterates Linearly through the bucket until the correct key is found. 
        TODO: should be able to use iterators with begin() and end() to iterate more easily through the list.
        for(auto& node : list) { ... } 
        OR
        for(iterator a = bucket.begin(); a != bucket.end(); a = a->next) { ... }
    */
    PNode* findInBucket(size_t idx, const_ref<Key> k) { /* internal members of b should exist in memory, unless you like seg faults. */
        if(buckets[idx].list == nullptr) 
            return nullptr;

        PNode* node  = buckets[idx].list;
        bool   found = false;
        while(node->next != nullptr && !found) {
            node  = node->next;
            found = compare_function(node->k, k);
        };

        /* if we didn't find the node, we want to nullify the pointer. else we keep it as it was. */
        node = __rcast(PNode*, __rcast(size_t, node) * found);
        return node;
    }


    void grow(size_t updatedBucketAmount);


public:
    void overrideFunctors(const_ref<HashFunction> hasher, const_ref<CompareFunction> comparer) {
        hash_function    = hasher;
        compare_function = comparer;
        return;
    }


	void create(u32 maxAmountOfNodes, u32 amountOfBuckets);
	void destroy();

	Value* lookup(const_ref<Key> k);
	bool   insert(const_ref<Value> v, const_ref<Key> k);
	bool   del(const_ref<Key> k);

    void to_stdout();
    void to_file(FILE* file);
};


/* Source: https://github.com/hhrhhr/MurmurHash-for-Lua/blob/master/MurmurHash64A.c */
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