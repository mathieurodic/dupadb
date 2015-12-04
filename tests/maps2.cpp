#include "File.hpp"


struct DupaHeader {
    char type[8];
    struct {
        uint8_t main;
        uint8_t revision;
        uint8_t release;
        uint8_t __filler;
    } version;
    char version_name[52];

    inline void set() {
        strncpy(type, "DUPADB", sizeof(type));
        strncpy(version_name, "Sunny Afternoon", sizeof(version_name));
        version = {
            .main = 0,
            .revision = 1,
            .release = 1,
        };
    }
    inline const bool check() const {
        return
            !strcmp(type, "DUPADB") &&
            !strcmp(version_name, "Sunny Afternoon") &&
            version.main == 0 &&
            version.revision == 1 &&
            version.release == 1;
    }
};


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



int main(int argc, char const *argv[]) {
    start();

    FilePager<
        CounterHeader<uint32_t>, uint32_t,
        4096, char[32], char[16],
        256
    > pager("storage/test_maps2", 1024*1024);

    for (int i=0; i<1024; i++) {
        auto& page = pager.get(i);
        sprintf(page.header, "%-31d", i);
        for (int j=0; j<253; j++) {
            sprintf(page.values[j], "%015d", j);
        }
    }

    finish(return);
}
