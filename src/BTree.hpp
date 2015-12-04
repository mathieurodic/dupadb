#ifndef __INCLUDED__BTree_hpp__
#define __INCLUDED__BTree_hpp__


#include "DupaDB.hpp"
#include "FileMap.hpp"


// File header for B-trees

template <typename size_t, size_t _key_size, size_t _block_size>
struct BTreeHeader : DupaHeader {

    char subtype[4];
    uint32_t key_size;
    uint32_t block_size;
    bool must_initialize;

    inline void set() {
        set_dupa();
        memcpy(subtype, "BTR+", 4);
        key_size = _key_size;
        block_size = _block_size;
        must_initialize = true;
    }
    inline const bool check() {
        return
            check_dupa() &&
            !memcmp(subtype, "BTR+", 4) &&
            (key_size == _key_size) &&
            (block_size == _block_size);
    }

};


// Nodes (file block) for B-trees

template <typename size_t, size_t key_size, size_t block_size>
struct BTreeNode {

    static const size_t MAX_CHILD_NUMBER;

    // flags
    bool is_root;
    bool is_leaf;
    // neighbours
    size_t parent;
    size_t next;
    size_t prev;
    // keys & indices
    size_t keys_length;
    char keys[MAX_CHILD_NUMBER][key_size];
    size_t children[MAX_CHILD_NUMBER];

    inline void split() {
    }
    inline void insert(const void* key, size_t value) {
    }

};

template <typename size_t, size_t key_size, size_t block_size>
const size_t BTreeNode<size_t, key_size, block_size>::MAX_CHILD_NUMBER = (block_size - 2 * sizeof(bool) - 4 * sizeof(size_t)) / (key_size + sizeof(size_t));


// The B-tree itself

template <typename size_t, size_t key_size, size_t reserve_size=1024*1024, size_t block_size=4096, size_t block_cache_length=16>
struct BTree : FileMap<BTreeHeader<size_t, key_size, block_size>, BTreeNode<size_t, key_size, block_size>, size_t, reserve_size, block_size, block_cache_length> {

    BTreeNode<size_t, key_size, block_size>& root;

    inline BTree(const char* file_path) :
        FileMap<
            BTreeHeader<size_t, key_size, block_size>,
            BTreeNode<size_t, key_size, block_size>,
            size_t, reserve_size, block_size, block_cache_length
        >(file_path),
        root((*this)[0])
    {
        if (this->header->must_initialize) {
            root.is_root = true;
            root.is_leaf = true;
            root.keys_length = 0;
            this->header->must_initialize = false;
        }
    }

};


#endif // __INCLUDED__BTree_hpp__
