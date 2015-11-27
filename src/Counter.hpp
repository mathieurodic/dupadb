#ifndef __INCLUDED__Counter_hpp__
#define __INCLUDED__Counter_hpp__


#include "DupaDB.hpp"
#include "FileMap.hpp"


template <typename size_t>
struct CounterHeader : DupaHeader {
    char subtype[4];
    uint32_t intsize;
    size_t counter;

    inline void set() {
        set_dupa();
        memcpy(subtype, "CNTR", 4);
        intsize = sizeof(size_t);
        counter = 0;
    }
    inline const bool check() {
        return
            check_dupa() &&
            !memcmp(subtype, "CNTR", 4) &&
            (intsize == sizeof(size_t));
    }

};

template <typename value_t, typename size_t, bool must_copy_id=false, size_t id_offset=0, size_t reserve_size=1048576, size_t block_size=4096, size_t block_cache_maxcount=16>
struct Counter : FileMap<CounterHeader<size_t>, value_t, size_t, reserve_size, block_size, block_cache_maxcount> {

    size_t& counter;

    inline Counter(const char* _file_path) :
        FileMap<CounterHeader<size_t>, value_t, size_t, reserve_size, block_size, block_cache_maxcount>(_file_path),
        counter(this->header->counter) {}

    inline const bool reserve(size_t length) {
        return this->set_file_size(((counter + length) /
            FileMap<CounterHeader<size_t>, value_t, size_t, reserve_size, block_size, block_cache_maxcount>::block_capacity
        + 2) * block_size);
    }

    inline size_t insert(const value_t& value) {
        size_t id = ++this->header->counter;
        if (must_copy_id) {
            * (size_t*) ((char*)(&value) + id_offset) = id;
        }
        memcpy(&(*this)[id], &value, sizeof(value_t));
        return id;
    }

};


#endif // __INCLUDED__Counter_hpp__
