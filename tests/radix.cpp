#include "util/logging.hpp"

#include <vector>
#include <string>


template <typename size_t>
struct RadixTreeBlock {

    struct node_t {
        size_t child_block_index;
        size_t value_index;
    };
    node_t nodes[256];

    RadixTreeBlock() {
        memset(this, 0, sizeof(*this));
        debug("initialized block with %zu bytes", sizeof(*this));
    }

};


template <typename size_t>
struct RadixTree {

    typedef RadixTreeBlock<size_t> block_t;
    std::vector<block_t> blocks;

    inline RadixTree() {
        blocks.resize(1);
    }

    inline typename block_t::node_t& find(const char* key_data, const size_t& key_size) {
        static typename block_t::node_t node_start = {.child_block_index=0, .value_index=0};
        typename block_t::node_t& node = node_start;
        const char* data = key_data;
        for (size_t depth=0; depth<key_size; depth++) {
            block_t& block = blocks[node.child_block_index];
            node = block.nodes[*data++];
            if (node.child_block_index == 0) {
                return node;
            }
        }
        return node;
    }
    inline block_t& find(const std::string& key) const {
        return find(key.data(), key.size());
    }

    inline bool insert(const char* key_data, const size_t key_size, const size_t value_index) {
        typename block_t::node_t& node = find(key_data, key_size);

        return true;
    }
    inline bool insert(const std::string& key, const size_t value_index) {
        return insert(key.data(), key.size(), value_index);
    }
};


int main(int argc, char const *argv[]) {
    start();

    RadixTree<uint32_t> tree;
    tree.insert("un", 1);
    tree.insert("deux", 2);
    tree.insert("trois", 3);
    tree.insert("quatre", 4);
    tree.insert("cinq", 5);
    tree.insert("six", 6);
    tree.insert("sept", 7);
    tree.insert("huit", 8);
    tree.insert("neuf", 9);
    tree.insert("dix", 10);
    tree.insert("onze", 11);
    tree.insert("douze", 12);
    tree.insert("treize", 13);
    tree.insert("quatorze", 14);
    tree.insert("quinze", 15);
    tree.insert("seize", 16);
    tree.insert("dix-sept", 17);
    tree.insert("dix-huit", 18);
    tree.insert("dix-neuf", 19);
    tree.insert("vingt", 20);
    tree.insert("cent", 100);
    tree.insert("mille", 1000);

    finish(return);
}
