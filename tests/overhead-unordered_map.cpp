#include "util/logging.hpp"

#include <unordered_map>
#include <unistd.h>


static const uint32_t n = 256*256*256;


int main(int argc, char const *argv[]) {
    start();

    message("insert %u elements", n);
    std::unordered_map<uint32_t, uint32_t> test;
    for (uint32_t i=0; i<n; i++) {
        test[rand()] = rand();
    }

    message("wait");
    char buffer[256];
    scanf("%s", buffer);

    message("check");
    uint32_t sum = 0;
    for (auto it=test.begin(); it!=test.end(); it++) {
        sum += it->second - it->first;
    }

    finish(return);
}
