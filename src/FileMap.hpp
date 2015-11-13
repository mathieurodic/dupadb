#ifndef __INCLUDED__FileMap_hpp__
#define __INCLUDED__FileMap_hpp__


#include "util/logging.hpp"

#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <deque>
#include <unordered_map>



template <typename value_t, typename size_t, size_t block_size>
struct FileBlock {

    value_t* values;
    static std::unordered_map<value_t*, uint16_t> pointers_count;
    static const size_t capacity;

    // construction & destruction
    inline FileBlock(const FileBlock& source) {
        values = source.values;
        ++pointers_count[values];
    }
    inline FileBlock(const int& file_handle, const size_t file_offset) {
        values = (value_t*) mmap(NULL, block_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_handle, file_offset);
        ++pointers_count[values];
    }
    inline ~FileBlock() {
        if (--pointers_count[values] == 0) {
            if (munmap(values, block_size) == -1) {
                error("could not unmap");
            }
            values = NULL;
        }
    }

    // flush data
    inline bool flush() {

    }

    // get/set data
    inline value_t& operator [] (size_t index) const {
        return values[index];
    }

    // debugging only
    inline void show() const {
        debug("<FileBlock @%lX max_index=%u>", (uint64_t)values, capacity);
    }

};

// initialization of static members

template <typename value_t, typename size_t, size_t block_size>
const size_t FileBlock<value_t, size_t, block_size>::capacity = block_size / sizeof(value_t);

template <typename value_t, typename size_t, size_t block_size>
std::unordered_map<value_t*, uint16_t> FileBlock<value_t, size_t, block_size>::pointers_count;



template <typename header_t, typename value_t, typename size_t, size_t reserve_size, size_t block_size, size_t block_cache_maxcount>
struct FileMap {

    // file-related information
    const char* file_path;
    int file_handle;
    size_t file_size;

    // file header
    header_t* header;

    // type used as block
    typedef FileBlock<value_t, size_t, block_size> block_t;
    // capacity of a block
    size_t block_cache_count;
    static const size_t block_capacity;

    // cache management
    std::unordered_map<size_t, block_t> block_cache;
    std::deque<size_t> block_cache_queue;

    // constructor
    inline FileMap(const char* _file_path) {
        // open file
        file_path = _file_path;
        file_handle = open(file_path, O_RDWR | O_CREAT | O_DSYNC, 0666);
        if (file_handle == -1) {
            fatal("could not open: `%s`", file_path);
        }
        // prepare if empty
        bool must_set_header = false;
        file_size = get_file_size();
        if (file_size == 0) {
            set_file_size(block_size);
            must_set_header = true;
        }
        header = (header_t*) mmap(NULL, sizeof(header_t), PROT_READ | PROT_WRITE, MAP_SHARED, file_handle, 0);
        if (must_set_header) {
            header->set();
        }
        // initialize values
        block_cache_count = 0;
    }
    inline ~FileMap() {
        if (munmap(header, sizeof(header_t)) == -1) {
            fatal("could not unmap: `%s`", file_path);
        }
        if (close(file_handle) == -1) {
            fatal("could not close: `%s`", file_path);
        }
    }

    // file management
    inline const bool reserve(size_t length) {
        return set_file_size((length / block_capacity + 2) * block_size);
    }
    inline const size_t get_file_size() {
        struct stat file_stat;
        if (fstat(file_handle, &file_stat) == -1) {
           fatal("error while reading stat for: `%s`", file_path);
        }
        return (file_size = file_stat.st_size);
    }
    inline const size_t set_file_size(const size_t& new_file_size) {
        static const char zero = '\0';
        if (file_size >= new_file_size) {
            return file_size;
        }
        size_t real_file_size = new_file_size + (reserve_size - new_file_size % reserve_size);
        if (lseek(file_handle, real_file_size - 1, SEEK_SET) == -1) {
            fatal("error while seeking file: `%s`", file_path);
        }
        if (write(file_handle, &zero, 1) == 0) {
            fatal("error while resizing file: `%s`", file_path);
        }
        file_size = real_file_size;
        return new_file_size;
    }

    // get/set data
    inline value_t& operator [] (size_t index) {
        // get indices
        size_t block_index = index / block_capacity;
        size_t block_value_index = index % block_capacity;
        // try to find in a listed block
        auto block_iterator = block_cache.find(block_index);
        if (block_iterator != block_cache.end()) {
            return block_iterator->second[block_value_index];
        }
        // otherwise, instanciate a block
        // ...destroy a block if maximum capacity has been reached
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
        set_file_size((block_index + 2) * block_size);
        block_t block(file_handle, (block_index + 1) * block_size);
        block_iterator = block_cache.insert(std::pair<size_t, block_t>(block_index, block)).first;
        block_cache_queue.push_back(block_index);
        return block_iterator->second[block_value_index];
    }


};

template <typename header_t, typename value_t, typename size_t, size_t reserve_size, size_t block_size, size_t block_cache_length>
const size_t FileMap<header_t, value_t, size_t, reserve_size, block_size, block_cache_length>::block_capacity = FileMap<header_t, value_t, size_t, reserve_size, block_size, block_cache_length>::block_t::capacity;


#endif // __INCLUDED__FileMap_hpp__
