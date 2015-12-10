#include "util/logging.hpp"
#include "BTree.hpp"

#include "generators.hpp"


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
    inline const str_t<size> operator = (const std::string source) {
        strncpy(_data, source.c_str(), size);
        return *this;
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

    for (int i=0; i<282; i++) {

        uint16_t value = i;
        key = number2expression(value);

        // btree.show();
        // message("INSERT: %s/%u", key._data, value);

        btree.insert(key, value);
    }

    message("show");
    // btree.show();
    message("iterate");
    uint64_t i = 0;
    for (auto it=btree.begin(); it!=btree.end(); ++it) {
        debug("%-6lu `%s` -> %u", i++, it.key().data(), it.value());
    }
    // btree.show();
    finish(return);
}
