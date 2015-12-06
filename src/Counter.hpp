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


template <typename value_t, typename size_t, size_t page_size>
struct CounterPage {
    static const size_t values_per_page;
    value_t values[values_per_page];
};

template <typename value_t, typename size_t, size_t page_size>
const size_t CounterPage<value_t, size_t, page_size>::values_per_page = page_size / sizeof(value_t);


template<
    typename value_t, typename size_t,
    size_t page_size, size_t pages_max_count
>
struct Counter : FilePager<
    CounterHeader<size_t>, size_t,
    page_size, CounterPage<value_t, size_t, page_size>,
    pages_max_count
> {

    static const size_t values_per_page;

    inline Counter(const char* path, size_t reserve_size) : FilePager<CounterHeader<size_t>, size_t, page_size, CounterPage<value_t, size_t, page_size>, pages_max_count>(path, reserve_size) {
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


#endif // __INCLUDED__Counter_hpp__
