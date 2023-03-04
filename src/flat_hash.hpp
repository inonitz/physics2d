#include "allocator.hpp"
#include <vector>
#include <functional>




template<typename Key, typename Value>
struct flat_hash
{
public:
    using HashFunction    = std::function<u64(Key)>;
    using CompareFunction = std::function<bool(Key, Key)>; /* MUST return 1 if keys match! */

#define INSERT_SUCCESS 0
#define INSERT_FAIL_TABLE_FULL 1
#define INSERT_FAIL_BUCKET_FULL 2

#define DELETE_SUCCESS 0
#define DELETE_FAIL_TABLE_EMPTY 1
#define DELETE_FAIL_BUCKET_EMPTY 2
#define DELETE_FAIL_KEY_NOT_FOUND 3

private:
	using ValueManager    = StaticPoolAllocator<Value, true>;
	static constexpr f32 cm_growth_factor = 1.5f;
    static constexpr f32 cm_load_factor   = 0.70f;


	size_t current_load() { return m_tableSize / m_buckets.size(); }


	constexpr size_t maxKeysPerBucket(size_t nodes, size_t buckets) {
		f32 out = __scast(f32, nodes) / buckets;
		return __scast(size_t, (out > 0.0f && out < 1.0f) + out); 
	}

	constexpr size_t minimumNodesFromBuckets(size_t buckets) {
		return __scast(size_t, __scast(f32, buckets) * (1 / cm_load_factor) );
	}




	struct alignpk(64) KeyGroup {
		struct pack {
			u8 bits; /* first 5 bits: occupation of each node. */
			u8 pad[3];
		} ctrl_t;
		struct {
			Key key;
			u32 index;
		} Node[5];


		KeyGroup() : ctrl_t{0b00000}, Node({ 
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 }
		}) {}

		__force_inline u8 first_available() const { return __builtin_ffs(~ctrl_t.bits) - 1; } /* get index to first available block.     */
		__force_inline u8 first_occupied()  const { return __builtin_ffs(ctrl_t.bits) - 1;  } /* make sure NOT to call when bits == 0 :) */
		bool full()  const { return ctrl_t.bits == 0b11111; }
		bool empty() const { return ctrl_t.bits == 0b00000; }
		void push(const_ref<Key> key, u32 idx)
		{
			u8 ins = first_available();
			Node[ins]	 = { key, idx };
			ctrl_t.bits |= (1 << ins); /* set bit to 1, meaning occupied. */
			return;
		}
	};


	u8 bucket_lookup(u32 bidx, const_ref<Key> key) {
		u8 i = 5;
		
		i *= (m_buckets[bidx].ctrl_t.bits == 0);
		while(cmp(key, m_buckets[bidx].Node[i].key) != true && i < 5) ++i;
		return i;
	}


	u32 bucket_del(u32 bidx, const_ref<Key> key) { /* assume the KeyGroup will ALWAYS contain atleast a single element before deletion. */
		u32 i = 0, valueIdx = 0;
		while(cmp(key, m_buckets[bidx].Node[i].key) != true && i < 5) ++i;


		if(unlikely(i == 5)) /* i.e not found. */
			return DEFAULT32;
		else {
			m_buckets[bidx].ctrl_t.bits &= ~(1 << i); /* clear bit[i] to 0, meaning free. */
			valueIdx = m_buckets[bidx].Node[i].index;

			m_buckets[bidx].Node[i] = { 0, DEFAULT32 };
			return valueIdx;
		}
	}


	u32 bucket_del_nocleanup(u32 bidx, const_ref<Key> key)
	{
		u32 i = 0;
		while( cmp(key, m_buckets[bidx].Node[i].key) != true && i < 5) ++i;

		if(unlikely(i == 5)) return DEFAULT32;

		m_buckets[bidx].ctrl_t.bits &= ~(1 << i);
		m_buckets[bidx].Node[i].key = 0;
		return m_buckets[bidx].Node[i].index;
	}



	/* [NOTE]: 
		Theres always the problem of, will the re-hash actually satisfy the requirement. 
		In most cases - it will. In VERY UNLIKELY cases, it won't.
		Thats why I'm letting the user take care of this >:)
	*/
	bool grow_buckets_rehash(size_t newSize) {
		std::vector<KeyGroup> newTable(newSize);
		u64 bidx = 0;
		u8  ctrl = 0, node;
		bool bucketNotFull = true;


		m_tableSize = 0;
		auto& bucket = m_buckets[0];
		for(size_t i = 0; bucketNotFull && i < m_buckets.size(); ++i) 
		{
			bucket = m_buckets[i];
			ctrl   = bucket.ctrl_t.bits;
			while(ctrl && bucketNotFull) { /* find every occupied node and copy it to its new position */
				node = bucket.first_occupied();
				bidx = hash(bucket.Node[node].key) % newTable.size();


				/* push first occupied node in current bucket (b) to new position in new table (newTable) */
				newTable[bidx].push(
					bucket.Node[node].key, 
					bucket.Node[node].value
				);
				++m_tableSize;

				bucketNotFull = !newTable[bidx].full();
				ctrl &= ~(1 << node); /* 'delete' the node from the bitmap, so we can go to next node. */
			}
		}
		m_rehashed += bucketNotFull;
		
		
		return bucketNotFull;
	}




private:
	HashFunction    hash;
	CompareFunction cmp;


	/* Value allocator & Value buffer (we only Own the pointer, we do not Manage it) */
	ValueManager 		  m_vmng;
	Value*                m_values;
	
	std::vector<KeyGroup> m_buckets;
	u32 				  m_tableSize;
    u8                    m_rehashed;
	u8 					  pad[3];


public:
    void overrideFunctors(const_ref<HashFunction> hasher, const_ref<CompareFunction> comparer) {
        hash = hasher;
        cmp  = comparer;
        return;
    }


	void create(u32 buckets, u32 suggestedNodeAmount = 0)
	{
		m_buckets.resize(buckets);
		
		suggestedNodeAmount += (suggestedNodeAmount == 0) * minimumNodesFromBuckets(buckets);
		m_values = __scast(Value*,   _mm_malloc(sizeof(Value) * suggestedNodeAmount, round2(sizeof(Value)))  );
		m_vmng.create(m_values, suggestedNodeAmount);

		m_tableSize = 0;
		m_rehashed  = 0;

		constexpr KeyGroup tmp = { 0b00000, {0, DEFAULT32}, {0, DEFAULT32}, {0, DEFAULT32}, {0, DEFAULT32}, {0, DEFAULT32} };
		std::fill(m_buckets.begin(), m_buckets.end(), tmp);
		return;
	}


	void destroy()
	{
		m_tableSize = 0;
		m_rehashed  = 0;
		m_buckets.clear();
		m_vmng.destroy();
		_mm_free(m_values);
		return;
	}

	
	Value* lookup(const_ref<Key> k) {
		u32 bidx = hash(k) % m_buckets.size(), vidx = 0;
		u8  nidx = m_buckets[bidx].lookup(k);
		bool found = (nidx != 5);


		/* instead of the if statement, we'll multiply each index value by the condition, that way we can avoid it. */
		bidx *= found; /* found=false: will give bucket 0 */
		nidx *= found; /* found=false: will give node   0 */
		vidx = found * m_buckets[bidx].Node[nidx].index; /* found=false: will give m_values[0] */
		return found * &m_values[vidx];					 /* found=false: will return nullptr   */
	}


	u8 insert(const_ref<Key> k, const_ref<Value> v)
	{
		if(unlikely(m_vmng.size() == m_tableSize)) /* Hashtable needs resize */
			return INSERT_FAIL_TABLE_FULL;

		u32 idx = hash(k) % m_buckets.size();
		if(unlikely(m_buckets[idx].full())) 
			return INSERT_FAIL_BUCKET_FULL; /* Hashtable needs resize */

		
		u32 vidx = m_vmng.allocate_index(); /* get new value from alloc */
		m_buckets[idx].push(k, vidx); 	    /* push into available position in bucket */
		m_values[vidx] = v; 		  	    /* set value to what we were given. */
		++m_tableSize;						/* table increased in size.         */


		// if(current_load() > cm_load_factor) { /* hash table re-hashing will be the users' job */
		// 	grow_buckets_rehash(__scast(size_t, __scast(f32, m_tableSize) * cm_growth_factor ));
		// }
		return INSERT_SUCCESS;
	}


	u8 del(const_ref<Key> k)
	{
		if(unlikely(m_tableSize == 0)) return DELETE_FAIL_TABLE_EMPTY;
		
		u32 idx = hash(k) % m_buckets.size();
		if(unlikely(m_buckets[idx].empty())) 
			return DELETE_FAIL_BUCKET_EMPTY;
		
		u32 vidx = m_buckets[idx].del(k);
		if(unlikely(vidx == DEFAULT32))
			return DELETE_FAIL_KEY_NOT_FOUND;


		m_vmng.free_index(vidx);
		return DELETE_SUCCESS;
	}


	bool rehash(size_t suggestedNewSize = 0)
	{
		size_t tmp = __scast(size_t, __scast(f32, m_tableSize) * cm_growth_factor );
		suggestedNewSize += (suggestedNewSize == 0) * tmp;
		return grow_buckets_rehash(suggestedNewSize);
	}


	bool   need_rehash() const { return current_load() > cm_load_factor; }
	bool   full() const  { return m_tableSize == m_vmng.size(); }
	bool   empty() const { return m_tableSize == 0; }
	size_t size() const  { return m_tableSize;      }
	Value* data() const  { return m_values;    	    }


	#undef INSERT_FAIL_BUCKET_FULL
	#undef INSERT_FAIL_TABLE_FULL
	#undef INSERT_SUCCESS
	#undef DELETE_FAIL_KEY_NOT_FOUND
	#undef DELETE_FAIL_BUCKET_EMPTY
	#undef DELETE_FAIL_TABLE_EMPTY
	#undef DELETE_SUCCESS
};