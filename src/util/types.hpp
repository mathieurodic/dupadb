#ifndef __INCLUDED__utils__types_hpp__
#define __INCLUDED__utils__types_hpp__


#include <unordered_map>


// fixed-sized character strings

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

namespace std {
    template<uint32_t size>
    struct hash<str_t<size>> {
        std::size_t operator()(const str_t<size>& str) const {
            const char* data = str.data();
            std::size_t hash = 31;
            while (*data) {
                hash = (hash * 54059) ^ (data[0] * 76963);
                data++;
            }
            return hash;
        }
    };
}



#endif // __INCLUDED__utils__types_hpp__
