#ifndef __INCLUDED__DupaDB_hpp__
#define __INCLUDED__DupaDB_hpp__


#include <stdint.h>
#include <string.h>


struct version_t {
    uint8_t main;
    uint8_t revision;
    uint16_t release;
    inline const bool operator ==(const version_t& other) {
        return !memcmp(this, &other, sizeof(version_t));
    }
};
static const version_t dupa_version = {
    .main = 0,
    .revision = 1,
    .release = 1,
};

struct DupaHeader {
    char type[4];
    version_t version;

    inline void set_dupa() {
        memcpy(type, "DUPA", 4);
        version = dupa_version;
    }
    inline const bool check_dupa() {
        return !memcmp(type, "DUPA", 4) && (version == dupa_version);
    }
};


#endif // __INCLUDED__DupaDB_hpp__
