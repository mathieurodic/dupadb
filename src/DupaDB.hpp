#ifndef __INCLUDED__DupaDB_hpp__
#define __INCLUDED__DupaDB_hpp__


#include <stdint.h>
#include <string.h>


struct version_t {
    union {
        struct {
            uint8_t main;
            uint8_t revision;
            uint8_t release;
            uint8_t __filler;
        };
        uint8_t __data[4];
    };
    inline const bool operator == (const version_t& other) {
        return !memcmp(this, &other, sizeof(version_t));
    }
    inline const bool operator >= (const version_t& other) const {
        for (int i=0; i<3; i++) {
            if (__data[i] > other.__data[i]) {
                return true;
            } else if (__data[i] < other.__data[i]) {
                return false;
            }
        }
        return true;
    }
};
static const version_t dupa_version = {
    .main = 0,
    .revision = 1,
    .release = 1,
    .__filler = 0,
};


struct DupaHeader {
    char type[8];
    version_t version;
    char version_name[52];

    inline void set() {
        strncpy(type, "DUPADB", sizeof(type));
        version = dupa_version;
        strncpy(version_name, "Sunny Afternoon", sizeof(version_name));
    }
    inline const bool check() const {
        return
            !strcmp(type, "DUPADB") &&
            version >= dupa_version;
    }
};


#include "File.hpp"


#endif // __INCLUDED__DupaDB_hpp__
