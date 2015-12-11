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

    inline bool operator != (const char* other) const {
        return strncmp(_data, other, size) != 0;
    }
    inline bool operator != (const std::string& other) const {
        return strncmp(_data, other.c_str(), size) != 0;
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

    for (uint64_t i=0; i<500; i++) {

        uint16_t value = i;
        key = number2expression(value);

        // btree.show();
        // message("INSERT: %s/%u", key._data, value);

        // btree.show();
        message("...%lu...", i);
        btree.insert(key, value);
        btree.show();

        bool found_error = false;
        if (i<10)continue;
        str_t<> previous_key;
        uint64_t count = 0;
        previous_key.clear();
        for (auto it=btree.begin(); it!=btree.end(); ++it) {
            debug("%-6lu `%s` -> %u", count, it.key().data(), it.value());
            count++;
            if (it.key() != number2expression(it.value())) {
                found_error = true;
                error("VALUE ERROR: `%s` != %u", it.key()._data, it.value());
            }
            if (previous_key >= it.key()) {
                found_error = true;
                error("ORDER ERROR: %s >= %s", previous_key._data, it.key()._data);
            }
            previous_key = it.key();
        }
        if (count != i + 1) {
            found_error = true;
            error("COUNT ERROR: %lu != %lu", count, i+1);
        }
        if (found_error) {
            printf("\n\n");
            btree.show();
            finish(return);
        }
    }


    uint64_t count = 0;
    for (auto it=btree.begin(); it!=btree.end(); ++it) {
        debug("%-6lu `%s` -> %u", count++, it.key().data(), it.value());
    }
    message("");
    // btree.show();
    finish(return);
}
