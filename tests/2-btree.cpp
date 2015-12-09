#include "util/logging.hpp"
#include "BTree.hpp"


template<uint32_t size>
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
    inline bool operator < (const str_t<size>& other) const {
        // notice("`%s` < `%s` : %s", _data, other._data, (memcmp(_data, other._data, size) < 0) ? "TRUE" : "FALSE");
        return (memcmp(_data, other._data, size) < 0);
    }
    inline bool operator <= (const str_t<size>& other) const {
        // notice("`%s` <= `%s` : %s", _data, other._data, (memcmp(_data, other._data, size) <= 0) ? "TRUE" : "FALSE");
        return (memcmp(_data, other._data, size) <= 0);
    }
    inline bool operator == (const str_t<size>& other) const {
        // notice("`%s` == `%s` : %s", _data, other._data, (memcmp(_data, other._data, size) == 0) ? "TRUE" : "FALSE");
        return (memcmp(_data, other._data, size) == 0);
    }
    inline bool operator >= (const str_t<size>& other) const {
        // notice("`%s` >= `%s` : %s", _data, other._data, (memcmp(_data, other._data, size) >= 0) ? "TRUE" : "FALSE");
        return (memcmp(_data, other._data, size) >= 0);
    }
    inline bool operator > (const str_t<size>& other) const {
        // notice("`%s` > `%s` : %s", _data, other._data, (memcmp(_data, other._data, size) > 0) ? "TRUE" : "FALSE");
        return (memcmp(_data, other._data, size) > 0);
    }
};


int main(int argc, char const *argv[]) {
    start();
    str_t<16> key;

    BTree<uint32_t, str_t<16>> btree("storage/test_2");

    for (int i=0; i<210; i++) {
        uint16_t value = i;
        snprintf(key._data, sizeof(key), "%x                  ", value);
        btree.insert(key, value);
    }

    btree.show();
    finish(return);
}
