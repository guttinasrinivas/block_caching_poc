cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(LRUFileCacheSimulation VERSION 1.0.0 LANGUAGES CXX)

# Force modern C++ standard requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Locate and configure the system multi-threading library (pthread, etc.)
find_package(Threads REQUIRED)

# Add the executable target (assumes your source code file is named main.cpp)
add_executable(lru_cache_sim src/lru_cache.cpp src/tests/unit_tests.cpp)

target_include_directories(lru_cache_sim PUBLIC src/inc src/tests/inc)

# Link the threads library to the target
target_link_libraries(lru_cache_sim PRIVATE Threads::Threads)

# Enable aggressive compiler warnings to keep the code tight and clean
if(MSVC)
    target_compile_options(lru_cache_sim PRIVATE /W4)
else()
    target_compile_options(lru_cache_sim PRIVATE -Wall -Wextra -Wpedantic)
endif()

