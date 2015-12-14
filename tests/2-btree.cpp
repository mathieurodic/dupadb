#include "util/logging.hpp"
#include "BTree.hpp"

#include "generators.hpp"

#include <map>


template<uint32_t size=256>
struct str_t {
    char _data[size];
    inline str_t() {
        memset(_data, 0, size);
    }
    inline str_t(const char* source) {
        strncpy(_data, source, size);
    }
    inline str_t(const str_t<size>& source) {
        strncpy(_data, source._data, size);
    }

    inline const int comp(const str_t<size>& other) const {
        return memcmp(_data, other._data, size);
    }

    inline bool operator != (const char* other) const {
        return strncmp(_data, other, size) != 0;
    }
    inline bool operator != (const std::string& other) const {
        return strncmp(_data, other.c_str(), size) != 0;
    }

    inline const bool operator < (const str_t<size>& other) const {
        return (comp(other) < 0);
    }
    inline const bool operator <= (const str_t<size>& other) const {
        return (comp(other) <= 0);
    }
    inline const bool operator == (const str_t<size>& other) const {
        return (comp(other) == 0);
    }
    inline const bool operator >= (const str_t<size>& other) const {
        return (comp(other) >= 0);
    }
    inline const bool operator > (const str_t<size>& other) const {
        return (comp(other) > 0);
    }
    inline const str_t<size>& operator = (const str_t<size>& source) {
        memcpy(_data, source._data, size);
        return *this;
    }
    inline const str_t<size>& operator = (const char* source) {
        strncpy(_data, source, size);
        return *this;
    }
    inline const str_t<size>& operator = (const std::string source) {
        strncpy(_data, source.c_str(), size);
        return *this;
    }
    inline void clear() {
        memset(_data, 0, size);
    }
    inline const char* data() const {
        return _data;
    }
};


int main(int argc, char const *argv[]) {
    start();
    str_t<> key;
    BTree<uint32_t, str_t<>> btree("storage/test_2");
    message("%u KEYS PER PAGE", btree.max_keys_count);
    message("%lu BYTES PER PAGE", sizeof(BTreePage<uint32_t, str_t<>, 4096>));

    std::map<str_t<>, uint32_t> key2value;
    for (uint64_t i=0; i<1024*1024; i++) {
        // message("...%lu...", i);
        if (i && i % 1024 == 0) {
            // message("...%lu...", i);
            // if (!btree.show_check(key2value)) {
            //     btree.show_pages();
            //     finish(return);
            // }
        }


        uint32_t value = i;
        key = number2expression(value);

        btree.insert(key, value);
        key2value[key] = value;

    }
    if (!btree.show_check(key2value)) {
        btree.show_pages();
        finish(return);
    }

    finish(return);
}
