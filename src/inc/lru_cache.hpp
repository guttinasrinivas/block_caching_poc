#ifndef __LRU_CACHE_HPP__
#define __LRU_CACHE_HPP__
#include <memory>
#include <map>
#include <vector>
#include <mutex>
#include <cassert>


// Configuration Constants
/* Ideally, this should be fetched from low level API */
#ifndef FILE_BLOCK_SIZE
#define FILE_BLOCK_SIZE (4096)
#endif /* FILE_BLOCK_SIZE */


constexpr size_t STAGING_THRESHOLD = 5;  // Reduced threshold for testing visibility

using FileOffst_t = uint64_t;
using Inode_t = uint64_t;
using Timestamp_t = uint64_t;
using CacheKey_t = uint64_t;
using BufPtr_t = std::shared_ptr<const uint8_t[]>;

struct CacheNode_t {
    CacheKey_t Key;
    Timestamp_t aTime;
    FileOffst_t fbn;
    Inode_t inode;
    BufPtr_t Buf;
};


/* TODO Preallocate cache nodes for the cache capacity, and track them */
class LRU_FileCache {
private:
    size_t Capacity;
    std::mutex cache_mutex;
    std::map<CacheKey_t, int> cache_tree;
    std::map<Timestamp_t, int> lru_tree;
    std::vector<int> staging_buffer;
    std::vector<CacheNode_t> CacheNodes;
    int CacheNodeIdx;

public:
    LRU_FileCache(size_t capacity) : Capacity(capacity),
                                    CacheNodes(capacity),
                                    CacheNodeIdx(0) {}

public:
    Timestamp_t get_current_time(void);
	void flush_staging_buffer(void);
	int evict(void);
    bool get(Inode_t inode, FileOffst_t fbn, BufPtr_t& out_data);
    void put(Inode_t inode, FileOffst_t fbn, BufPtr_t data);
    size_t size(void);

protected:
    void insert_node(CacheKey_t key, Inode_t inode, FileOffst_t fbn, BufPtr_t data, Timestamp_t now);

public:
    uint64_t derive_cache_key(Inode_t inode, FileOffst_t fbn) {
        uint64_t key = ((inode << 32) ^ ((fbn / FILE_BLOCK_SIZE) & 0xffffffff));
        assert (key > 0);
        return key;
    }
    
};


using LRU_FileCachePtr_t = std::shared_ptr<LRU_FileCache>;

#endif /* __LRU_CACHE_HPP__ */

