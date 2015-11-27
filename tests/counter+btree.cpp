#include "Counter.hpp"
#include "util/generators.hpp"

#include <vector>


template<typename key_t, typename size_t, size_t block_size=4096>
struct BTreeNode {

    struct child_t {
        key_t key;
        uint8_t is_leaf : 1;
        size_t index : sizeof(size_t) - 1; // index of value if leaf, or of child node otherwise
    };

    size_t children_count;
    child_t children[(block_size - sizeof(size_t)) / sizeof(child_t)];

    BTreeNode() {
        children_count = 0;
    }

};


template<typename key_t, typename size_t, size_t block_size=4096>
struct BTree {

    typedef BTreeNode<key_t, size_t, block_size> node_t;
    struct position_t {
        int result;
        size_t node_index;
        size_t child_index;
        typename node_t::child_t child;
    };

    std::vector<node_t> nodes;

    BTree() {
        nodes.push_back(node_t());
    }

    inline position_t split(const size_t& node_index) {
        // retrieve the old node, plus a brand new one
        nodes.push_back(node_t());
        node_t& node1 = nodes[node_index];
        node_t& node2 = nodes[nodes.size() - 1];

    }
    inline position_t find(const key_t& key) const {
        position_t position = {.is_exact=false, .node_index=0};
        while (true) {
            const node_t node = nodes[position.node_index];
            for (position.child_index=0; position.child_index<node.children_count; position.child_index++) {
                position.child = node.children[position.child_index];
                position.result = memcmp(&position.child.key, &key, sizeof(key_t));
                if (position.result >= 0) {
                    if (position.child.is_leaf) {
                        return position;
                    }
                    position.child_index = position.child.index;
                    break;
                }
            }
        }
    }
    inline position_t insert(const key_t& key, size_t data_index) {
        position_t position = find(key);
    }

};


struct Model {
    uint32_t id;
    uint16_t type_id;
    char name[26];
    char description[224];

    inline Model(const uint16_t& _type_id, const char* _name, const char* _description) {
        id = 0;
        type_id = _type_id;
        strncpy(name, _name, sizeof(name));
        strncpy(description, _description, sizeof(description));
    }

    inline void show() const {
        debug("<Model id=%u type_id=%hu name=`%.*s`>", id, type_id, (int) sizeof(name), name);
    }

};



int main(int argc, char const *argv[]) {

    start();

    message("test counter") {

        uint32_t id;
        uint32_t n = 16 * 65536;

        notice("instanciate counter")
            Counter<Model, uint32_t, true, 0, 16*1024*1024, 4096, 16> counter("storage/test_counter+btree");

        notice("append an instance") {
            Model instance(2, "hello world", "this is nothing but a simple test...");
            instance.show();
            id = counter.insert(instance);
            debug("inserted with id = %u", id);
            instance.show();
        }

        notice("retrieve the instance") {
            counter[id].show();
        }

        notice("insert %u elements with reservation", n) {
            debug("reserve");
            counter.reserve(n);
            debug("insert");
            for (uint32_t i=0; i<n; i++) {
                Model instance(i & 8, generate_gibberish<26>(), generate_gibberish<224>());
                id = counter.insert(instance);
                // instance.show();
            }
        notice("insert %u elements without reservation", n);
            debug("insert");
            for (uint32_t i=0; i<n; i++) {
                Model instance(i & 8, generate_gibberish<26>(), generate_gibberish<224>());
                id = counter.insert(instance);
                // instance.show();
            }
        }
    }

    finish(return);
}
