#ifndef __INCLUDED__BTree_hpp__
#define __INCLUDED__BTree_hpp__


#include "DupaDB.hpp"
#include "FilePager.hpp"


// File header for B-trees

template <typename size_t, typename key_t, size_t _page_size>
struct BTreeHeader {

    DupaHeader dupa;
    char subtype[8];
    uint32_t key_size;
    uint32_t page_size;
    uint32_t page_count;
    bool must_initialize;

    inline void set() {
        dupa.set();
        memcpy(subtype, "FIXDBTR+", 8);
        key_size = sizeof(key_t);
        page_size = _page_size;
        page_count = 0;
        must_initialize = true;
    }
    inline const bool check() {
        return
            dupa.check() &&
            !memcmp(subtype, "FIXDBTR+", 8) &&
            (key_size == sizeof(key_t)) &&
            (page_size == _page_size);
    }

};


// Nodes (file block) for B-trees

template <typename size_t, typename key_t, size_t page_size>
struct BTreePage {

    static const size_t max_keys_count;

    // header
    struct header_t {
        // flags
        bool is_leaf;
        size_t index;
        size_t parent_index;
        size_t keys_count;
    };
    header_t header;
    // keys & indices
    key_t keys[max_keys_count];
    size_t values[max_keys_count + 1];
    //
    inline const bool is_full() const {
        return header.keys_count >= max_keys_count;
    }
    inline const size_t find(const key_t& key) const {
        for (int i=0; i<header.keys_count; i++) {
            if (key > keys[i]) {
                for (int j=i+1; j<header.keys_count; j++) {
                    if (keys[j] > key) {
                        return j;
                    }
                }
                return header.keys_count;
            }
        }
        return 0;
    }
    // insertion
    inline const bool insert_at(const size_t index, const key_t& key, const size_t value) {
        size_t keys_count = header.keys_count++;
        if (keys_count && index < keys_count) {
            memmove(keys + index + 1, keys + index, sizeof(key_t) * (keys_count - index));
            memmove(values + index + 1, values + index, sizeof(size_t) * (keys_count - index));
        }
        keys[index] = key;
        values[index] = value;
        return true;
    }
    inline const bool insert(const key_t& key, const size_t value) {
        if (!header.is_leaf) {
            return false;
        }
        const size_t index = find(key);
        return insert_at(index, key, value);
    }
};

template <typename size_t, typename key_t, size_t page_size>
const size_t BTreePage<size_t, key_t, page_size>::max_keys_count = (page_size - sizeof(header_t) - sizeof(size_t)) / (sizeof(key_t) + sizeof(size_t));


// The B-tree itself

template <
    typename size_t, typename key_t,
    size_t reserve_size=1024*1024,
    size_t page_size=4096, size_t pages_max_count=256
>
struct BTree : FilePager<BTreeHeader<size_t, key_t, page_size>, size_t, page_size, BTreePage<size_t, key_t, page_size>, pages_max_count> {

    static const size_t max_keys_count;
    typedef BTreePage<size_t, key_t, page_size> page_t;

    inline BTree(const char* file_path) :
        FilePager<
            BTreeHeader<size_t, key_t, page_size>, size_t,
            page_size, BTreePage<size_t, key_t, page_size>, pages_max_count
        >(file_path, reserve_size)
    {
        if (this->header->must_initialize) {
            this->new_page();
        }
    }

    inline page_t& new_page(const size_t parent_index=0) {
        size_t page_index;
        page_t& page = this->get_page(page_index = this->header->page_count++);
        page.header = {
            .is_leaf = true,
            .index = page_index,
            .parent_index = parent_index,
            .keys_count = 0
        };
        return page;
    }

    inline void split(page_t& page) {
        static size_t split_index = max_keys_count / 2;
        if (page.header.index == 0) {
            // first child node
            page_t& child1_page = new_page();
            memcpy(child1_page.keys, page.keys, split_index * sizeof(key_t));
            memcpy(child1_page.values, page.values, split_index * sizeof(size_t));
            child1_page.header.keys_count = split_index;
            // second child node
            page_t& child2_page = new_page();
            memcpy(child2_page.keys, page.keys + split_index, (max_keys_count - split_index) * sizeof(key_t));
            memcpy(child2_page.values, page.values + split_index, (max_keys_count - split_index) * sizeof(size_t));
            child2_page.header.keys_count = max_keys_count - split_index;
            // root node
            memcpy(page.keys, child1_page.keys + split_index - 1, sizeof(key_t));
            page.values[0] = child1_page.header.index;
            page.values[1] = child2_page.header.index;
            page.header.keys_count = 1;
            page.header.is_leaf = false;
        }
    }
    inline bool insert(const key_t& key, size_t value) {
        size_t page_index = 0;
        while (true) {
            page_t& page = this->get_page(page_index);
            if (page.header.is_leaf) {
                break;
            }
            page_index = page.values[page.find(key)];
        }
        page_t& page = this->get_page(page_index);
        if (page.is_full()) {
            split(page);
        }
        if (page.insert(key, value)) {
            return true;
        }
        return false;
    }

    inline void show(size_t page_index=0, size_t depth=0) {
        if (page_index == 0) {
            debug("BTREE");
        }
        const page_t& page = this->get_page(page_index);
        std::string prefix(4 * (depth + 1), '.');
        if (page.header.is_leaf) {
            for (uint32_t i=0, n=page.header.keys_count; i<n; i++) {
                debug("%s [%02u-%04u] `%s` -> %u", prefix.c_str(), page_index, i, page.keys[i]._data, page.values[i]);
            }
        } else {
            for (uint32_t i=0, n=page.header.keys_count; i<=n; i++) {
                show(page.values[i], depth + 1);
                if (i != n) {
                    debug("%s [%02u-%04u] `%s`", prefix.c_str(), page_index, i, page.keys[i]._data);
                }
            }
        }
    }

};

template <typename size_t, typename key_t, size_t reserve_size, size_t page_size, size_t pages_max_count>
const size_t BTree<size_t, key_t, reserve_size, page_size, pages_max_count>::max_keys_count = BTreePage<size_t, key_t, page_size>::max_keys_count;


#endif // __INCLUDED__BTree_hpp__
