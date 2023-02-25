#include "hashtable.hpp"




template<typename Key, typename Value> void Hashtable<Key, Value>::create(
    u32 maxNodes,
    u32 bucketCount
) {
    nodes.create(maxNodes);
    values.create(maxNodes);
    buckets.resize(bucketCount);

    for(size_t i = 0; i < buckets.size(); ++i) {
        buckets[i] = { nullptr, nullptr, 0 };
    }
    return;
}


template<typename Key, typename Value> void Hashtable<Key, Value>::destroy()
{
    nodes.destroy();
    values.clear();
    buckets.clear();
    return;
}


template<typename Key, typename Value> Value* Hashtable<Key, Value>::lookup(const_ref<Key> key)
{
    u64    index  = hash_function(key) % buckets.size();
    PNode* node = findInBucket(index, key);

    return node->v;
}



/* I could probably re-write this in a more-concise way, first finish writing this goddamn abomination */
template<typename Key, typename Value> bool Hashtable<Key, Value>::insert(
    const_ref<Value> value, 
    const_ref<Key>   key
) {
    u64     index  = hash_function(key) % buckets.size();
    PNode*  node;
    Value*  tmp;
    Bucket& bucket = buckets[index];


    /* Bucket needs memory assigned to it. */
    if(bucket.list == nullptr) {
        bucket.list = bucket.last = nodes.allocate();
        tmp = values.allocate();
        *tmp = value;

        bucket.last->value = { key, tmp };
        bucket.last->next = nullptr;
        ++bucket.length;
        ++tableSize;
        return true;
    }

    /* Node needs to be found in bucket, if exists we'll override the original value, and if doesn't we'll append it. */
    node = findInBucket(index, key);
    if(node != nullptr) /* key was found */
    {
        node->value.k = key;
        node->value.v = value;
    } else { /* key doesn't exist in bucket, we create it and increment bucket->last */
        node = nodes.allocate();
        tmp  = values.allocate();
        node->value.k = key;
        node->value.v = tmp;
        *tmp = value;
        bucket.last->next = node;
        bucket.last = bucket->last->next;
        ++bucket.length;
        ++tableSize;
    }


    /* Check if the table needs to be re-distributed and re-hashed. */
    f32 newSize = loadFactor();
    if(newSize > m_load_factor) {
        newSize = m_growth_factor * loadFactor();
        grow(__scast(size_t, newSize));
        /* TODO: Need to also rehash everything so we'll deal with that later. */
    }
    return true;
}


/* 
    TODO: Test.
*/
template<typename Key, typename Value> bool Hashtable<Key, Value>::del(const_ref<Key> key)
{
    u64    index = hash_function(key) % buckets.size();
    PNode* node  = buckets[index].list;
    bool   found = false;
    
    if(node == nullptr) return false;
    
    
    found = compare_function(node->k, key);
    while(node->next != nullptr && !found) {
        node  = node->next;
        found = compare_function(node->next.k, key);
    }
    if(!found) 
        return false;
    

    PNode copyOfDeleted = *node->next; /* get a copy of all data in the deleted node.            */
    node->next = copyOfDeleted.next;   /* node->next = node->next->next | skip the deleted node. */
    node->next->next = nullptr;        /* unlink node->next from the linked list.                */
    nodes.free(node->next);       /* return the memory of the node  */
    values.free(copyOfDeleted.v); /* return the memory of the value */

    --buckets[index].length;
    --tableSize;
    return true;
}


