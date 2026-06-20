#ifndef __LRU_CACHE_HPP__
#define __LRU_CACHE_HPP__
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <random>
#include <cassert>
#include <thread>

// Configuration Constants
constexpr size_t FILE_BLOCK_SIZE = 4096; // 4KB Blocks
constexpr size_t CACHE_CAPACITY = 100;   // Reduced capacity to trigger eviction quickly
constexpr size_t STAGING_THRESHOLD = 5;  // Reduced threshold for testing visibility

using FBN = uint64_t;         
using Inode = uint64_t;       
using Timestamp = uint64_t;   

struct CacheKey {
    Inode inode;
    FBN fbn;

    bool operator<(const CacheKey& other) const {
        if (inode != other.inode) return inode < other.inode;
        return fbn < other.fbn;
    }
};

struct CacheBlock {
    CacheKey key;
    std::vector<uint8_t> data;
    Timestamp last_accessed;
};


class LRUFileCache {
private:
    std::mutex cache_mutex;
    std::map<CacheKey, CacheBlock> cache_tree;
    std::map<Timestamp, CacheKey> lru_tree;
    std::vector<CacheKey> staging_buffer;

public:

    Timestamp get_current_time(void);
	void flush_staging_buffer(void);
	void evict(void);
    bool get(Inode inode, FBN fbn, std::vector<uint8_t>& out_data);
    void put(Inode inode, FBN fbn, const std::vector<uint8_t>& data);
    size_t size(void);
};


#endif /* __LRU_CACHE_HPP__ */

