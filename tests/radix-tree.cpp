#include <string.h>
#include <string>
#include <vector>

#include <stdint.h>


template <typename key_t, typename value_t, typename size_t, size_t key_offset>
struct RadixTree {

    struct node_t {

        union {
            size_t is_internal__index;
            struct {
                uint8_t is_internal : 1;
                size_t index : sizeof(size_t) - 1;
            };
        };
        node_t() : is_internal__index(0) {}
        inline void show() const {
            printf("<node internal=%c index=%u>\n", is_internal ? 't' : 'f', index);
        }
        operator const bool () const {
            return is_internal__index != 0;
        }
    };

    struct lookup_result_t {
        size_t depth;
        size_t block_index;
        size_t node_index;
        inline lookup_result_t(const lookup_result_t& source) {
            memcpy(this, &source, sizeof(*this));
        }
        inline lookup_result_t() {
            memset(this, 0, sizeof(*this));
        }
        inline lookup_result_t(const size_t _depth, const size_t _block_index, const size_t _node_index) : depth(_depth), node_index(_node_index), block_index(_block_index) {}
        inline void show() const {
            printf("<result depth=%u block_index=%u node_index=%u>\n",
                depth,
                block_index,
                node_index
            );
        }
    };

    struct block_t {
        node_t nodes[256];
        inline block_t() {
            memset(this, 0, sizeof(*this));
        }
        inline node_t& operator [] (const uint8_t node_index) {
            return nodes[node_index];
        }
    };

    static node_t null_node;
    std::vector<block_t> blocks;
    std::vector<value_t> values;

    inline RadixTree() {
        blocks.resize(1);
        values.resize(1);
    }

    inline static key_t& value2key(value_t& value) {
        return * (key_t*) ((char*)&value + key_offset);
    }
    inline static const key_t& value2key(const value_t& value) {
        return * (key_t*) ((const char*)&value + key_offset);
    }

    inline lookup_result_t lookup(const key_t& key) {
        const char* key_data = (const char*) &key;
        lookup_result_t result;
        result.node_index = key_data[0];
        while (result.depth < sizeof(key)) {
            const node_t& node = blocks[result.block_index][result.node_index];
            if (node.is_internal == 0) {
                return result;
            }
            result.depth++;
            result.block_index = node.index;
            result.node_index = key_data[result.depth];
        }
        // this should not happen
        return result;
    }

    inline const bool find(const key_t& key, value_t& value) {
        lookup_result_t result = lookup(key);
        block_t& result_block = blocks[result.block_index];
        node_t& result_node = result_block[result.node_index];
        if (result_node) {
            const size_t suffix_size = sizeof(key_t) - result.depth;
            value = values[result_node.index];
            value.show();
            if (suffix_size && memcmp((const char*)&key, &value2key(value), suffix_size)) {
                return false;
            }
            return true;
        }
        return false;
    }

    inline const bool has(const key_t& key, const lookup_result_t& result) {
        if (result.node) {
            const size_t suffix_size = sizeof(key_t) - result.depth;
            const value_t& value = values[result.node.value_index];
            if (suffix_size == 0) {
                return true;
            }
            if (memcmp((const char*)&key, &value2key(value), suffix_size)) {
                return true;
            }
            return false;
        }
        return false;
    }
    inline const bool has(const key_t& key) {
        lookup_result_t result = lookup(key);
        return has(key, result);
    }

    inline const bool insert(const value_t& value) {
        const key_t& key = value2key(value);
        lookup_result_t result = lookup(key);
        node_t& result_node = blocks[result.block_index].nodes[result.node_index];

        printf("\n");
        value.show();
        result.show();

        // exit if key exists
        if (result.depth == sizeof(key)) {
            printf("DUPLICATE\n");
            return false;
        }
        if (memcmp((const char*) &(values[result_node.index]) + key_offset + result.depth, (const char*)&key + result.depth, sizeof(key) - result.depth) == 0) {
            printf("DUPLICATE\n");
            return false;
        }

        {
            node_t& result_node = blocks[result.block_index].nodes[* ((const unsigned char*) &(key) + result.depth)];
            // if the result node points to nothing... yet!
            if (result_node.index == 0) {
                result_node.index = values.size();
                values.push_back(value);
                show();
                return true;
            }
            // otherwise, the result node becomes an internal node, and its value goes to a leaf node together with the new one
            else {
                // create new leaf block
                size_t leaf_block_index = blocks.size();
                blocks.resize(leaf_block_index + 1);
                block_t& leaf_block = blocks[leaf_block_index];
                // node leaf is copied from result leaf
                value_t& result_value = values[result_node.index];
                uint8_t c1 = * ((const unsigned char*) &(result_value) + key_offset + result.depth + 1);
                leaf_block.nodes[c1].index = result_node.index;
                // node leaf is created
                uint8_t c2 = * ((const unsigned char*) &(key) + result.depth + 1);
                printf("%u\n", values.size());
                leaf_block.nodes[c2].index = values.size();
                values.push_back(value);
                // former leaf block becomes internal
                block_t& result_block = blocks[result.block_index];
                result_block.nodes[* ((const unsigned char*) &(key) + result.depth)].is_internal = 1;
                result_block.nodes[* ((const unsigned char*) &(key) + result.depth)].index = leaf_block_index;
                show();
                return true;
            }

        }

        // otherwise, it points to a value


        // TODO: move previous value node, read pointer to next character

        // add new block
        size_t block_index = blocks.size();
        blocks.resize(block_index + 1);
        block_t& block = blocks[block_index];
        // initialize leaf node in that block
        node_t& node = block[((const unsigned char*)&key)[result.depth + 1]];
        node.index = values.size();
        values.push_back(value);
        // change result from leaf to internal
        blocks[result.block_index].nodes[result.node_index].is_internal = 1;
        blocks[result.block_index].nodes[result.node_index].index = block_index;


        printf("\n");
        show();
        printf("\n\n\n");

        return true;
    }

    inline void show(const size_t block_index=0, const size_t depth=0) const {
        const block_t& block = blocks[block_index];
        if (block_index) {
            printf("block #%u\n", block_index);
        }
        for (int n=0; n<256; n++) {
            const node_t& node = block.nodes[n];
            if (!node) {
                continue;
            }
            for (size_t d=0; d<depth; d++) {
                printf("    ");
            }
            printf("%c: ", n);
            if (node.is_internal) {
                show(node.index, depth + 1);
            } else {
                values[node.index].show();
            }
        }

        // for (int j=0; j<blocks.size(); j++) {
        //     printf("<block #%u>\n", j);
        //     const block_t& block = blocks[j];
        //     for (int i=0; i<256; i++) {
        //         const node_t& node = block.nodes[i];
        //         if (node) {
        //             if (node.is_internal) {
        //                 printf("\t<node %-3u|%c block_index=%u>\n", i, i, node.index);
        //             } else {
        //                 printf("\t<node %-3u|%c value_index=%u>\n\t\t", i, i, node.index);
        //                 values[node.index].show();
        //             }
        //         }
        //     }
        // }
        // printf("\n");
    }

};



struct Model {
    uint32_t id;
    char name[16];
    inline Model() {
        memset(this, 0, sizeof(*this));
    }
    inline Model(const Model& source) {
        memcpy(this, &source, sizeof(*this));
    }
    inline Model(const uint32_t _id, const char* _name) {
        id = _id;
        strncpy(name, _name, sizeof(name));
    }
    inline void show() const {
        printf("<Model id=%u name=%s>\n", id, name);
    }
};


#ifndef offsetof
#define offsetof(C, P) (size_t)(((C*)(NULL))->P)
#endif


int main(int argc, char const *argv[]) {
    RadixTree<char[16], Model, uint32_t, offsetof(Model, name)> tree;

    tree.show();

    tree.insert(Model(1, "un"));
    // {
    //     tree.show();
    //     return 0;
    // }
    tree.insert(Model(2, "deux"));
    tree.insert(Model(3, "trois"));
    tree.insert(Model(4, "quatre"));
    tree.insert(Model(5, "cinq"));
    tree.insert(Model(6, "six"));
    tree.insert(Model(7, "sept"));
    tree.insert(Model(8, "huit"));
    tree.insert(Model(9, "neuf"));
    tree.insert(Model(10, "dix"));
    tree.insert(Model(11, "onze"));
    tree.insert(Model(12, "douze"));
    // printf("\n\n\n");



    Model model;
    char key[16];
    strncpy(key, "un", 16);
    if (tree.find(key, model)) {
        model.show();
    }
    return 0;
}
