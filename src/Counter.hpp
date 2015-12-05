#ifndef __INCLUDED__Counter_hpp__
#define __INCLUDED__Counter_hpp__


#include "DupaDB.hpp"
#include "FilePager.hpp"


template <typename size_t>
struct CounterHeader {

    DupaHeader dupa;
    char reserved[8];
    size_t intsize;
    size_t counter;

    inline void set() {
        dupa.set();
        memcpy(reserved, "FIXDCNTR", 8);
        intsize = sizeof(size_t);
        counter = 0;
    }
    inline const bool check() {
        return
            dupa.check() &&
            !memcmp(reserved, "FIXDCNTR", 8) &&
            intsize == sizeof(size_t);
    }
};


template<
    typename value_t, typename size_t,
    size_t page_size, size_t pages_max_count
>
struct Counter : FilePager<
    CounterHeader<size_t>, size_t,
    page_size, char[0], value_t,
    pages_max_count
> {

    static const size_t values_per_page;

    inline Counter(const char* path, int reserve_size) : FilePager<CounterHeader<size_t>, size_t, page_size, char[0], value_t, pages_max_count>(path, reserve_size) {
    }

    inline const size_t append(const value_t& value) {
        const size_t counter = this->header->counter++;
        if (counter == -1) {
            return 0;
        }
        memcpy(
            this->get_page(counter / values_per_page).values + (counter % values_per_page),
            &value,
            sizeof(value_t)
        );
        return counter + 1;
    }

};

template<
    typename value_t, typename size_t,
    size_t page_size, size_t pages_max_count
>
const size_t Counter<value_t, size_t, page_size, pages_max_count>::values_per_page = page_size / sizeof(value_t);

/*
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
*/

#endif // __INCLUDED__Counter_hpp__
