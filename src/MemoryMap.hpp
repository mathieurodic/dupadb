#ifndef __INCLUDED__MemoryMap_hpp__
#define __INCLUDED__MemoryMap_hpp__


#include "util/logging.hpp"

#include <stdlib.h>

#include <deque>
#include <unordered_map>


template <typename value_t, typename size_t, size_t block_size>
struct MemoryBlock {

    value_t* values;
    static std::unordered_map<value_t*, uint16_t> pointers_count;
    static const size_t capacity;

    // construction & destruction
    inline MemoryBlock(const MemoryBlock& source) {
        values = source.values;
        ++pointers_count[values];
    }
    inline MemoryBlock() {
        values = (value_t*) malloc(block_size);
        ++pointers_count[values];
    }
    inline ~MemoryBlock() {
        if (--pointers_count[values] == 0) {
            free(values);
            values = NULL;
        }
    }

    // getters
    inline value_t& operator [] (size_t index) const {
        return values[index];
    }

    // debugging only
    inline void show() const {
        debug("<MemoryBlock @%lX max_index=%u>", (uint64_t)values, capacity);
    }

};

// initialization of static members

template <typename value_t, typename size_t, size_t block_size>
const size_t MemoryBlock<value_t, size_t, block_size>::capacity = block_size / sizeof(value_t);

template <typename value_t, typename size_t, size_t block_size>
std::unordered_map<value_t*, uint16_t> MemoryBlock<value_t, size_t, block_size>::pointers_count;



template <typename value_t, typename size_t, size_t block_size, size_t block_cache_maxcount>
struct MemoryMap {

    // type used as block
    typedef MemoryBlock<value_t, size_t, block_size> block_t;
    // capacity of a block
    size_t block_cache_count;
    static const size_t block_capacity;

    // cache management
    std::unordered_map<size_t, block_t> block_cache;
    std::deque<size_t> block_cache_queue;

    // constructor
    inline MemoryMap() {
        block_cache_count = 0;
    }

    // get block
    inline block_t& get_block(const size_t block_index) {
        // try to find a cached block
        auto block_iterator = block_cache.find(block_index);
        if (block_iterator != block_cache.end()) {
            return block_iterator->second;
        }
        // otherwise, instanciate a block
        // ...destroy a block if maximum cache capacity has been reached
        if (++block_cache_count > block_cache_maxcount) {
            const size_t& oldest_block_index = * (block_cache_queue.begin());
            block_cache_queue.pop_front();
            block_iterator = block_cache.find(oldest_block_index);
            if (block_iterator != block_cache.end()) {
                block_cache.erase(block_iterator);
            }
            block_cache_count--;
        }
        // ...instanciate the required block
        block_t block;
        block_iterator = block_cache.insert(std::pair<size_t, block_t>(block_index, block)).first;
        block_cache_queue.push_back(block_index);
        return block_iterator->second;
    }

    // get/set values
    inline value_t& operator [] (size_t index) {
        return get_block(index / block_capacity)[index % block_capacity];
    }

};

template <typename value_t, typename size_t, size_t block_size, size_t block_cache_length>
const size_t MemoryMap<value_t, size_t, block_size, block_cache_length>::block_capacity = MemoryMap<value_t, size_t, block_size, block_cache_length>::block_t::capacity;


#endif // __INCLUDED__MemoryMap_hpp__
