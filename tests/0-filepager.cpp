#include "DupaDB.hpp"
#include "util/logging.hpp"


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
    > pager("storage/test_0", 1024*1024);

    for (int i=0; i<1024; i++) {
        auto& page = pager.get(i);
        sprintf(page.header, "%-31d", i);
        for (int j=0; j<253; j++) {
            sprintf(page.values[j], "%015d", j);
        }
    }

    finish(return);
}
