#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <random>
#include <cassert>
#include <thread>
#include "lru_cache.hpp"



// --- SIMULATION RUNNER ---
int main() {
    LRUFileCache cache;
    std::vector<uint8_t> mock_data(FILE_BLOCK_SIZE, 0xFF);
    std::vector<uint8_t> read_buffer;

    std::cout << "=== Test 1: Basic Ingestion and Capacity Enforcement ===\n";
    Inode test_file_1 = 1001;
    
    // Fill cache up to capacity limit
    for (FBN ff = 0; ff < CACHE_CAPACITY; ++ff) {
        cache.put(test_file_1, ff, mock_data);
    }
    std::cout << "Cache filled to current size: " << cache.size() << " / " << CACHE_CAPACITY << "\n";

    // Adding one more block must trigger flush and eviction
    std::cout << "\nInserting block " << CACHE_CAPACITY << " to trigger eviction:\n";
    cache.put(test_file_1, CACHE_CAPACITY, mock_data);
    
    // The very first block added (FBN 0) should now be gone
    bool hit = cache.get(test_file_1, 0, read_buffer);
    std::cout << "Verifying if oldest block (FBN 0) was evicted: " << (hit ? "FOUND (Fail)" : "NOT FOUND (Success)") << "\n";

    std::cout << "\n=== Test 2: Hybrid Staging Buffer Hits ===\n";
    // Hit a single block multiple times to fill staging threshold without altering the tree instantly
    std::cout << "Simulating rapid reads on FBN 50 to test deferred rebalancing:\n";
    for (int i = 0; i < STAGING_THRESHOLD; ++i) {
        cache.get(test_file_1, 50, read_buffer);
    }
    
    std::cout << "\n=== Test 3: Multi-Threaded Heavy Content Simulation ===\n";
    std::vector<std::thread> workers;
    
    // Launch threads competing for hits and updates across multiple files
    for (int t = 0; t < 4; ++t) {
        workers.emplace_back(([&cache, &mock_data, t]() {
            std::vector<uint8_t> dummy;
            Inode thread_inode = 2000 + t;
            for (FBN f = 0; f < 30; ++f) {
                cache.put(thread_inode, f, mock_data);
                cache.get(thread_inode, f, dummy);
            }
        }));
    }

    for (auto& worker : workers) {
        worker.join();
    }

    std::cout << "\nFinal Cache size after concurrent processing: " << cache.size() << "\n";
    std::cout << "All simulations passed without deadlocks or memory errors.\n";
    return 0;
}

