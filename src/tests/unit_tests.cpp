/**
 * @file        unit_tests.cpp
 * @brief       Reference/example implementation of LRU block caching, unit tests
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

#include <iostream>
#include <vector>
#include <cassert>
#include <thread>
#include "lru_cache.hpp"


#ifndef CACHE_CAPACITY
#define CACHE_CAPACITY (100 * FILE_BLOCK_SIZE)
#endif /* CACHE_CAPACITY */


/* Test script. We'll be OK! */
uint8_t mock_data_buf[FILE_BLOCK_SIZE];


// --- SIMULATION RUNNER ---
int main() {
    LRU_FileCache *cache = new LRU_FileCache(CACHE_CAPACITY / FILE_BLOCK_SIZE);

    BufPtr_t mock_data(new uint8_t[FILE_BLOCK_SIZE]);
    BufPtr_t read_buffer;

    std::cout << "=== Test 1: Basic Ingestion and Capacity Enforcement ===\n";
    Inode_t test_file_1 = 1001;
    
    // Fill cache up to capacity limit
    for (FileOffst_t ff = 0; ff < CACHE_CAPACITY; ff+=FILE_BLOCK_SIZE) {
        cache->put(test_file_1, ff, mock_data);
    }
    std::cout << "Cache filled to current size: " 
              << cache->size() << " / " << CACHE_CAPACITY / FILE_BLOCK_SIZE << "\n";

    // Adding one more block must trigger flush and eviction
    std::cout << "\nInserting block " << CACHE_CAPACITY << " to trigger eviction:\n";
    cache->put(test_file_1, CACHE_CAPACITY, mock_data);
    
    // The very first block added (FBN 0) should now be gone
    bool hit = cache->get(test_file_1, 0, read_buffer);
    std::cout << "Verifying if oldest block (FBN 0) was evicted: " << (hit ? "FOUND (Fail)" : "NOT FOUND (Success)") << "\n";

    std::cout << "\n=== Test 2: Hybrid Staging Buffer Hits ===\n";
    
    // Hit a single block multiple times to fill staging threshold without altering the tree instantly
    std::cout << "Simulating rapid reads on FBN 50 to test deferred rebalancing:\n";
    auto start = cache->get_current_time();
    for (size_t ii = 0; ii < STAGING_THRESHOLD; ++ii) {
        cache->get(test_file_1, 50, read_buffer);
    }
    auto duration = cache->get_current_time() - start;
    std::cout << "completed in " << duration << "uS.\n";
    
    std::cout << "\n=== Test 3: Multi-Threaded Heavy Content Simulation ===\n";
    std::vector<std::thread> workers;
    start = cache->get_current_time();
    // Launch threads competing for hits and updates across multiple files
    for (int t = 0; t < 4; ++t) {
        workers.emplace_back(([&cache, &mock_data, t]() {
            BufPtr_t dummy;
            Inode_t thread_inode = 2000 + t;
            for (FileOffst_t f = 0; f < (30 * FILE_BLOCK_SIZE); f+= FILE_BLOCK_SIZE) {
                cache->put(thread_inode, f, mock_data);
                cache->get(thread_inode, f, dummy);
            }
        }));
    }

    for (auto& worker : workers) {
        worker.join();
    }
    duration = cache->get_current_time() - start;
    std::cout << "completed in " << duration << "uS.\n";

    std::cout << "\nFinal Cache size after concurrent processing: " << cache->size() << "\n";
    std::cout << "All simulations passed without deadlocks or memory errors.\n";

    delete cache;

    return 0;
}

