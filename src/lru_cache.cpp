#include <stdio.h>
#include "lru_cache.hpp"


Timestamp LRUFileCache::get_current_time(void) {
    // Sleep slightly to guarantee unique timestamps in fast test iterations
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

void LRUFileCache::flush_staging_buffer(void) {
    std::cout << "[LRU Tree Sync] Flushing " << staging_buffer.size() << " keys from staging buffer.\n";
    for (const auto& key : staging_buffer) {
        auto it = cache_tree.find(key);
        if (it != cache_tree.end()) {
            lru_tree.erase(it->second.last_accessed);
            it->second.last_accessed = get_current_time();
            lru_tree[it->second.last_accessed] = key;
        }
    }
    staging_buffer.clear();
}

void LRUFileCache::evict(void) {
    if (lru_tree.empty()) return;

    auto oldest_it = lru_tree.begin();
    CacheKey key_to_evict = oldest_it->second;

    std::cout << "[Eviction Triggered] Dropping Inode: " << key_to_evict.inode
              << ", FBN: " << key_to_evict.fbn << "\n";

    cache_tree.erase(key_to_evict);
    lru_tree.erase(oldest_it);
}

bool LRUFileCache::get(Inode inode, FBN fbn, std::vector<uint8_t>& out_data) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    CacheKey key{inode, fbn};

    auto it = cache_tree.find(key);
    if (it == cache_tree.end()) {
        return false;
    }

    out_data = it->second.data;
    staging_buffer.push_back(key);

    if (staging_buffer.size() >= STAGING_THRESHOLD) {
        flush_staging_buffer();
    }

    return true;
}

void LRUFileCache::put(Inode inode, FBN fbn, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    CacheKey key{inode, fbn};
    Timestamp now = get_current_time();

    auto it = cache_tree.find(key);
    if (it != cache_tree.end()) {
        it->second.data = data;
        staging_buffer.push_back(key);
        if (staging_buffer.size() >= STAGING_THRESHOLD) {
            flush_staging_buffer();
        }
    } else {
        if (cache_tree.size() >= CACHE_CAPACITY) {
            flush_staging_buffer();
            evict();
        }

        CacheBlock new_block{key, data, now};
        cache_tree[key] = new_block;
        lru_tree[now] = key;
    }
}

size_t LRUFileCache::size(void) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    return cache_tree.size();
}


