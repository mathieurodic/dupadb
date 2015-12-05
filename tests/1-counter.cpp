#include "DupaDB.hpp"
#include "util/logging.hpp"


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
