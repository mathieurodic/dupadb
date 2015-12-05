#include "DupaDB.hpp"
#include "util/logging.hpp"


int main(int argc, char const *argv[]) {
    start();

    Counter<char[17], uint32_t, 4096, 256> cntr("storage/test_1a", 1024*1024);

    for (int i=0; i<4096; i++) {
        char v[17];
        sprintf(v, "%-16d", i);
        cntr.append(v);
    }

    Counter<char[16], uint32_t, 4096, 256> counter("storage/test_1b", 1024*1024);
    // uint32_t n = 1024*1023;
    uint32_t n = 64*1024*1024;
    notice("insert %u things", n);
    char value[16];
    strncpy(value, ":-)\n", 16);
    for (uint32_t i=0; i<n; i++) {
        counter.append(value);
    }

    finish(return);
}
