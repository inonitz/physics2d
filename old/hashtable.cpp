#include "hashtable.hpp"




template<typename Key, typename Value, bool rehashOnCondition> void Hashtable<Key, Value, rehashOnCondition>::rehash(size_t newSize)
{
    std::vector<Bucket> newBuckets; newBuckets.resize(newSize);
    Node* begin = nullptr;
    u64   index = 0;
    for(auto& bucket : buckets)
    {
        for(begin = bucket.begin(); begin != nullptr; begin = begin->next) {
            index = hash_function(begin->k) % newBuckets.size();
            newBuckets[index].append(begin);
        }
    }
    buckets.swap(newBuckets); /* newBuckets is the actual buckets we need. buckets (old) will be freed once out of scope. */
    return;
}




template<typename Key, typename Value, bool rehashOnCondition> void Hashtable<Key, Value, rehashOnCondition>::create(
    u32 maxNodes,
    u32 bucketCount
) {
    nodes.create(maxNodes);
    values.create(maxNodes);
    buckets.resize(bucketCount);
    m_tableSize = 0.0f;
    m_maxBucketLength = { 0, 0 };
    return;
}


template<typename Key, typename Value, bool rehashOnCondition> void Hashtable<Key, Value, rehashOnCondition>::destroy()
{
    nodes.destroy();
    values.destroy();
    buckets.clear();
    return;
}


template<typename Key, typename Value, bool rehashOnCondition> Value* Hashtable<Key, Value, rehashOnCondition>::lookup(const_ref<Key> key)
{
    u64   index = hash_function(key) % buckets.size();
    Node* node  = findInBucket(index, key);

    return node == nullptr ? nullptr : node->v;
}


template<typename Key, typename Value, bool rehashOnCondition> std::pair<u8, Value*> Hashtable<Key, Value, rehashOnCondition>::internal_insert(
    const_ref<Key>   key,
    const_ref<Value> value
) {
    if(unlikely(m_tableSize == nodes.size())) {
        return { false, nullptr };
    }

    u64 index = hash_function(key) % buckets.size();
    Node*  node = findInBucket(index, key);
    Value* tmp;

    /* key already exists, we'll update the value. */
    if(node != nullptr) {
        *node->v = value;
        return { 2u, node->v };
    }


    /* we didn't find the node in the appropriate bucket. we need to insert it. */
    node = nodes.allocate();
    node->k = key;
    node->v = values.allocate();
    buckets[index].append(node);
    ++m_tableSize;

    /* 
        In the case where we exceed max_nodes_per_bucket (ex. 4) OR the load factor is too high, 
        we'll increase the amount of buckets and rehash the Whole Hashtable. 
    */
    m_maxBucketLength.first  = (buckets[index].length > m_maxBucketLength) ? buckets[index].length : m_maxBucketLength;
    m_maxBucketLength.second = (buckets[index].length > m_maxBucketLength) ? index                 : m_maxBucketLength.second;
    if(tableShouldGrow()) {
        f32 newSize = __scast(f32, buckets.size()) * cm_growth_factor;
        rehash(__scast(size_t, newSize));
    }
    return { true, node->v }; 
}


template<typename Key, typename Value, bool rehashOnCondition> u8 Hashtable<Key, Value, rehashOnCondition>::insert(
    const_ref<Key>   key,
    const_ref<Value> value
) {
    return internal_insert(key, value).first;
}


template<typename Key, typename Value, bool rehashOnCondition> bool Hashtable<Key, Value, rehashOnCondition>::del(const_ref<Key> key)
{
    bool t = deleteFromBucket(hash_function(key) % buckets.size(), key);
    m_tableSize -= t; /* if deletion occured, we'll subtract from tableSize. */
    return t;
}


template<typename Key, typename Value, bool rehashOnCondition> Value& Hashtable<Key, Value, rehashOnCondition>::operator[](const_ref<Key> key)
{
    auto pair = internal_insert(key, Value());
    ifcrashdbg(unlikely(pair.second == nullptr));
    return *pair.second;
}


template<typename Key, typename Value, bool rehashOnCondition> void Hashtable<Key, Value, rehashOnCondition>::to_file(FILE* file)
{
    fprintf(file, "Hashtable:\n    %u Buckets\n    %u/%u Nodes [%u Allocated (%u Bytes)]\n    %u Collisions\n    Bucket %u Largest [%u Nodes]\n    Contents:\n");
    Node* begin = nullptr;
    for(size_t id = 0; id < buckets.size(); ++id)
    {
        fprintf(file, "    [%llu]\n      %llu Nodes\n      begin %llX\n      end %llX\n      ", id, buckets[id].length, buckets[id].list, buckets[id].last);
        for(begin = buckets[id].begin(); begin != nullptr; begin = begin->next) {
            fprintf(file, "[%llu, %llX] => ", begin->k, begin->v);
        }
        fprintf(file, "||\n");
    }
    return;
}





