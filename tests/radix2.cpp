#include "util/logging.hpp"

#include <stdint.h>

#include <vector>


template <typename size_t, size_t page_size>
struct RadixTree {

    struct node_t {
        size_t length;
        uint8_t is_internal : 1;
        size_t index : 8 * sizeof(size_t) - 1;
        char data[];
    };

    static node_t null_node;

    struct page_t {
        union {
            struct {
                uint16_t size;
                uint16_t nodes_count;
            };
            char _[4096];
        };
        inline page_t() {
            memset(_, 0, page_size);
            size = sizeof(size) + sizeof(nodes_count);
        }
        inline bool append(const node_t* node, const size_t& node_length) {
            size_t node_size = sizeof(node_t) + node_length;
            if (size + node_size > page_size) {
                return false;
            }
            memcpy(_ + size, node, node_size);
            size += node_size;
            nodes_lengthes[nodes_count++] += node_length;
        }
        inline node_t& find(const char* data, const size_t size) {
            node_t& node = * (node_t*) (_ + sizeof(size) + sizeof(nodes_count));
            for (size_t i=0; i<nodes_count; i++) {
                if (memcmp(node))
            }
            return null_node;
        }
    };

    std::vector<page_t> pages;

    inline RadixTree() {
        pages.resize(1);
    }

    inline void find(const char* string_data, size_t string_size) {
        page_t* page = &pages[0];
        // node_t* node = (page._ + page.);
        while (true) {
            for (size_t i=0; i<page->length; i++) {

            }
        }
    }

};


int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
