#include "allocator.hpp"
#include <vector>
#include <functional>




template<typename Key, typename Value>
struct flat_hash
{
public:
    using HashFunction    = std::function<u64(Key)>;
    using CompareFunction = std::function<bool(Key, Key)>; /* MUST return 1 if keys match! */
private:
	using ValueManager    = StaticPoolAllocator<Value, true>;


	struct alignpk(64) KeyGroup {
		struct pack {
			u8 bits; /* first 5 bits: occupation of each node. */
			u8 pad[3];
		} ctrl_t;
		struct {
			u64 key;
			u32 index;
		} Node[5];

	};

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


	void grow_buckets_rehash(size_t newSize) {
		std::vector<KeyGroup> newTable; newTable.resize(newSize);
		u64 bidx = 0;

		m_tableSize = 0;
		for(auto& b : m_buckets) {
			
		}
		++m_rehashed;
		return;
	}




private:
	HashFunction    hash;
	CompareFunction cmp;


	ValueManager 		  m_vmng;
	std::vector<KeyGroup> m_buckets;
	Value*                m_values;
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
		
		suggestedNodeAmount = (suggestedNodeAmount != 0) * suggestedNodeAmount + (suggestedNodeAmount == 0) * minimumNodesFromBuckets(buckets);
		m_values = _mm_malloc(sizeof(Value) * suggestedNodeAmount, round2(sizeof(Value)));
		m_vmng.create(m_values, suggestedNodeAmount);

		m_tableSize = 0;
		m_rehashed  = 0;
	}


	void destroy()
	{
		m_buckets.clear();
		m_vmng.destroy();
		_mm_free(m_values);
		return;
	}

	
	Value* lookup(const_ref<Key> k) {
		u32 idx  = hash(k) % m_buckets.size();
		u8  tmp = m_buckets[idx].bits;
		
		if(tmp == 0) return nullptr;

		u8 i = 0;
		while(cmp( m_buckets[idx].Node[i].key, k )) ++i;
		return m_values[idx + m_buckets[idx].Node[i].index];
	}


	u8 insert(const_ref<Key> k, const_ref<Value> v)
	{
		if(unlikely(m_vmng.size() == m_tableSize)) return false;

		u32 idx = hash(k) % m_buckets.size();
		u8  tmp = m_buckets[idx].bits;

		if(unlikely(tmp == 0b11111)) 
			return false; /* can't insert into bucket. hashtable needs resize */

		u8 ins = __builtin_ffs(~tmp) - 1; /* get index to first available block. */
		m_buckets[idx].Node[ins] = { k, m_vmng.allocate_index() };
		m_buckets[idx].bits |= (1 << ins);
		++m_tableSize;


		if(current_load() > cm_load_factor) { 
			grow_buckets_rehash(__scast(size_t, __scast(f32, m_tableSize) * cm_growth_factor ));
		}
		return true;
	}


	u8 del(const_ref<Key> k);
};