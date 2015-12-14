#include "util/logging.hpp"
#include "util/generators.hpp"
#include "util/types.hpp"

#include "BTree.hpp"



int main(int argc, char const *argv[]) {

    start();
    str_t<> key;
    uint64_t n = 1024 * 1024;

    message("initialize BTree");
    BTree<uint32_t, str_t<>> btree("storage/test_2");
    notice("%u keys per page", btree.max_keys_count);
    notice("%lu bytes per page", sizeof(BTreePage<uint32_t, str_t<>, 4096>));

    message("insert many");
    std::unordered_map<str_t<>, uint32_t> key2value;
    for (uint64_t value=0; value<n; value++) {
        key = number2expression(value);
        btree.insert(key, value);
    }
    message("test");
    notice("insert into map");
    for (uint64_t value=0; value<n; value++) {
        key = number2expression(value);
        key2value[key] = value;
    }
    notice("compare map with BTree");
    if (!btree.show_check(key2value)) {
        btree.show_pages();
        finish(return);
    }

    finish(return);
}
