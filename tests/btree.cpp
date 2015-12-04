#include "BTree.hpp"
#include "util/logging.hpp"


int main(int argc, char const *argv[]) {
    start();

    BTree<int, 4> bt("storage/test_btree");

    finish(return);
}
