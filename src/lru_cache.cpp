/**
 * @file        lru_cache.cpp
 * @brief       Reference/example implementation of LRU block caching
 *
 * @author      Vasu (Srinivas Guttina)
 * @date        2026-02-28
 * @version     0.1
 *
 * @copyright   Copyright (c) 2026 Srinivas Guttina.
 *  
 * @license     GNU AGPL with exceptions. See LICENSE.md for further details
 *              and exceptions
 */

#include <stdio.h>
#include <thread>
#include "lru_cache.hpp"


Timestamp_t LRU_FileCache::get_current_time(void) {
    using std::chrono::microseconds;
    using std::chrono::duration_cast;
    using std::chrono::steady_clock;

    // Sleep slightly to guarantee unique timestamps in fast test iterations
    std::this_thread::sleep_for(microseconds(1));
    auto now = steady_clock::now();
    return duration_cast<microseconds>(now.time_since_epoch()).count();
}

void LRU_FileCache::flush_staging_buffer(void) {
    //std::cout << "[LRU Tree Sync] Flushing " << staging_buffer.size() << " keys from staging buffer.\n";
    
    for (const auto& idx : staging_buffer) {
        auto atime = CacheNodes[idx].aTime;
        lru_tree.erase(atime);
        lru_tree[atime] = idx;
    }

    staging_buffer.clear();
}

int LRU_FileCache::evict(void) {
    int ret = Capacity;

    if (lru_tree.empty()) {
        /* We should not be here!!! */
        //std::cout << "Warning: Trying to evict from an empty LRU";
        return ret;
    }

    auto oldest_it = lru_tree.begin();

    /* Let's ensure the node exists in the Cache */
    auto idx = oldest_it->second;
    assert((idx >= 0) && (idx < (int) Capacity));
    
    CacheNode_t &node = CacheNodes[idx];
    auto ct_it = cache_tree.find(node.Key);

    if (ct_it == cache_tree.end()) {
        //std::cout << "ERROR: node " << node.Key << " not found in the Cache, but exists in LRU.";
    } else {
        //std::cout << "[Eviction Triggered] Dropping node: " << node.Key
        //    << ", Inode: " << CacheNodes[idx].inode
        //        << ", FBN: " << CacheNodes[idx].fbn << "\n";
        cache_tree.erase(ct_it);
        ret = idx;
    }

    lru_tree.erase(oldest_it);

    return ret;
}

bool LRU_FileCache::get(Inode_t inode, FileOffst_t fbn, BufPtr_t& out_data) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    CacheKey_t key = derive_cache_key(inode, fbn);

    auto it = cache_tree.find(key);
    if (it == cache_tree.end()) {
        return false;
    }

    auto idx = it->second;
    auto now = get_current_time();
    CacheNodes[idx].aTime = now;
    out_data = CacheNodes[idx].Buf;
    staging_buffer.push_back(key);

    if (staging_buffer.size() >= STAGING_THRESHOLD) {
        flush_staging_buffer();
    }

    return true;
}

void LRU_FileCache::put(Inode_t inode, FileOffst_t fbn, BufPtr_t data) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    CacheKey_t key = derive_cache_key(inode, fbn);
    auto now = get_current_time();

    auto it = cache_tree.find(key);
    if (it != cache_tree.end()) {

        /* This block exists in cache! Update... */
        //std::cout << "Updating node " << key;

        auto idx = it->second;
        CacheNodes[idx].Buf = data;
        CacheNodes[idx].aTime = now;
        CacheNodes[idx].fbn = fbn;
        CacheNodes[idx].inode = inode;
        CacheNodes[idx].Key = key;

        staging_buffer.push_back(key);
        if (staging_buffer.size() >= STAGING_THRESHOLD) {
            flush_staging_buffer();
        }

        return;
    }

    insert_node(key, inode, fbn, data, now);
    return;
}

void LRU_FileCache::insert_node(CacheKey_t key, Inode_t inode, FileOffst_t fbn, BufPtr_t data, Timestamp_t now) {
    /* We need to insert a new node. Let's check if we can get one. */
    
    int idx = 0;

    if (CacheNodeIdx < (int) Capacity) {
        idx = CacheNodeIdx;
        CacheNodeIdx++;
    } else {
        flush_staging_buffer();
        idx = evict();
    }

    assert((idx >= 0) && (idx < (int) Capacity));
    //std::cout << "Inserting new node: " << key << " at [" << idx << "].\n";
    CacheNodes[idx].Buf = data;
    CacheNodes[idx].aTime = now;
    CacheNodes[idx].fbn = fbn;
    CacheNodes[idx].inode = inode;
    CacheNodes[idx].Key = key;
    cache_tree[key] = idx;
    lru_tree[now] = key;
}

size_t LRU_FileCache::size(void) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    return cache_tree.size();
}


