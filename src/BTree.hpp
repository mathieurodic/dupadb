#ifndef __INCLUDED__BTree_hpp__
#define __INCLUDED__BTree_hpp__


#include "DupaDB.hpp"
#include "FilePager.hpp"

#include <vector>

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
        bool is_leaf : sizeof(size_t) / 2;
        bool is_root : sizeof(size_t) / 2;
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
        if (key < keys[0]) {
            return 0;
        }
        for (int i=1; i<header.keys_count; i++) {
            if (key < keys[i]) {
                return i;
            }
        }
        return header.keys_count;
    }
    // insertion
    inline const bool insert_at(const size_t index, const key_t& key, const size_t value) {
        size_t keys_count = ++header.keys_count;
        if (header.is_leaf) {
            // shift to the right, to make some space
            if (index < keys_count) {
                memmove(keys + index + 1, keys + index, sizeof(key_t) * (keys_count - index));
                memmove(values + index + 1, values + index, sizeof(size_t) * (keys_count - index));
            }
            // put the key/value pair at the right place
            keys[index] = key;
            values[index] = value;
        } else {
            // shift to the right, to make some space
            if (index < keys_count) {
                memmove(keys + index + 1, keys + index, sizeof(key_t) * (keys_count - index));
                memmove(values + index + 2, values + index + 1, sizeof(size_t) * (keys_count - index));
            }
            // put the key/value pair at the right place
            keys[index] = key;
            values[index + 1] = value;
        }
        return true;
    }
    inline const bool insert(const key_t& key, const size_t value) {
        return insert_at(find(key), key, value);
    }
};

template <typename size_t, typename key_t, size_t page_size>
// const size_t BTreePage<size_t, key_t, page_size>::max_keys_count = 3;
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
            this->new_page().header.is_root = true;
        }
    }

    inline page_t& new_page(const size_t parent_index=0) {
        size_t page_index;
        page_t& page = this->get_page(page_index = this->header->page_count++);
        page.header = {
            .is_leaf = true,
            .is_root = false,
            .index = page_index,
            .parent_index = parent_index,
            .keys_count = 0
        };
        return page;
    }

    inline void split(page_t& page, const size_t parent_index=0) {
        static const size_t split_left = max_keys_count / 2;
        static const size_t split_right = max_keys_count - split_left;
        const key_t& split_key = page.keys[split_left];
        //
        // show();
        //
        // warning("SPLITTING NOW: %u", page.header.index);
        // show(page.header.parent_index);
        if (page.header.is_leaf) {
            if (page.header.is_root) {
                notice("SPLIT %u: is_leaf && is_root", page.header.index);
                // first new child
                page_t& child1 = new_page(page.header.index);
                memcpy(child1.keys, page.keys, split_left * sizeof(key_t));
                memcpy(child1.values, page.values, split_left * sizeof(size_t));
                child1.header.keys_count = split_left;
                // second new child
                page_t& child2 = new_page(page.header.index);
                memcpy(child2.keys, page.keys + split_left, split_right * sizeof(key_t));
                memcpy(child2.values, page.values + split_left, split_right * sizeof(size_t));
                child2.header.keys_count = split_right;
                // original
                page.header.keys_count = 1;
                page.header.is_leaf = false;
                page.keys[0] = split_key;
                page.values[0] = child1.header.index;
                page.values[1] = child2.header.index;
            } else {
                notice("SPLIT %u: is_leaf && !is_root", page.header.index);
                // original page
                page.header.keys_count = split_left;
                // new sibling
                page_t& sibling = new_page(page.header.parent_index);
                memcpy(sibling.keys, page.keys + split_left, split_right * sizeof(key_t));
                memcpy(sibling.values, page.values + split_left, split_right * sizeof(size_t));
                sibling.header.keys_count = split_right;
                // parent
                page_t& parent = this->get_page(parent_index);
                parent.insert(split_key, sibling.header.index);
            }
        } else {
            if (page.header.is_root) {
                // first new child
                page_t& child1 = new_page(page.header.index);
                child1.header.is_leaf = false;
                child1.header.keys_count = split_left;
                memcpy(child1.keys, page.keys, split_left * sizeof(key_t));
                memcpy(child1.values, page.values, (split_left + 1) * sizeof(key_t));
                // second new child
                page_t& child2 = new_page(page.header.index);
                child2.header.is_leaf = false;
                child2.header.keys_count = split_right - 1;
                memcpy(child2.keys, page.keys + split_left + 1, (split_right - 1) * sizeof(key_t));
                memcpy(child2.values, page.values + split_left + 1, split_right * sizeof(key_t));
                // original
                page.header.keys_count = 1;
                page.keys[0] = split_key;
                page.values[0] = child1.header.index;
                page.values[1] = child2.header.index;
                notice("SPLIT %u: !is_leaf / is_root", page.header.index);
            } else {
                notice("SPLIT %u: !is_leaf / !is_root", page.header.index);
                // new sibling
                page_t& sibling = new_page(page.header.parent_index);
                sibling.header.is_leaf = false;
                sibling.header.keys_count = split_right - 1;
                memcpy(sibling.keys, page.keys + split_left + 1, (split_right - 1) * sizeof(key_t));
                memcpy(sibling.values, page.values + split_left + 1, split_right * sizeof(key_t));
                // original
                page.header.keys_count = split_left;
                // parent
                page_t& parent = this->get_page(parent_index);
                parent.insert(split_key, sibling.header.index);
            }
        }
        // show();
        // warning("READY TO INSERT?")
        // if (!check()) {
        //     error(" ");
        //     show(page.header.parent_index);
        //     fatal("This B-tree is not ordered anymore!");
        // }
    }

    inline const bool insert(page_t& page, const key_t& key, const size_t value) {
        return page.insert(key, value);
    }
    inline const bool insert(const key_t& key, const size_t value) {
        size_t parent_index = 0;
        size_t page_index = 0;
        while (true) {
            page_t& page = this->get_page(page_index);
            if (page.is_full()) {
                split(page, parent_index);
                parent_index = page_index = 0;
                continue;
            }
            if (page.header.is_leaf) {
                break;
            }
            parent_index = page_index;
            page_index = page.values[page.find(key)];
        }
        page_t& page = this->get_page(page_index);
        return insert(page, key, value);
    }

    struct cursor_t {
        BTree<size_t, key_t, reserve_size, page_size, pages_max_count>* _btree;
        //
        std::vector<page_t*> _pages;
        std::vector<size_t> _path;
        //
        size_t _index;
        page_t* _page;
        key_t* _key;
        size_t* _value;

        inline cursor_t() : _btree(NULL) {}
        inline cursor_t(BTree<size_t, key_t, reserve_size, page_size, pages_max_count>* btree) : _btree(btree) {
            size_t page_index = 0;
            _index = 0;
            while (true) {
                _page = & _btree->get_page(page_index);
                if (_page->header.keys_count == 0) {
                    _page = NULL;
                    return;
                }
                _pages.push_back(_page);
                _path.push_back(0);
                if (_page->header.is_leaf) {
                    _key = _page->keys;
                    _value = _page->values;
                    _index = 0;
                    return;
                }
                page_index = _page->values[_index];
            }
        }

        inline key_t& key() {
            return *_key;
        }
        inline size_t& value() {
            return *_value;
        }

        inline void show_path() {
            std::string result;
            char buffer[256];
            for (int i=0, n=_path.size(); i<n; i++) {
                snprintf(buffer, sizeof(buffer), "(%u-%u) ", _path[i], _pages[i]->header.index);
                result += buffer;
            }
            snprintf(buffer, sizeof(buffer), "%u", _index);
            result += buffer;
            notice("%s", result.c_str());
        }

        inline const bool operator != (const cursor_t& other) {
            return _path.size() != other._path.size();
            // return _path.size() != other._path.size()
            //     || memcmp(_path.data(), other._path.data(), _path.size() * sizeof(size_t));
        }
        inline void operator ++ () {
            show_path();
            while (true) {
                if (++_index < _page->header.keys_count) {
                    ++_key;
                    ++_value;
                    return;
                }
                _index = _path.back() + 1;
                _path.pop_back();
                _pages.pop_back();
                _page = _pages.back();
                do {
                    if (_index <= _page->header.keys_count) {
                        _page = & _btree->get_page(_page->values[_index]);
                        _pages.push_back(_page);
                        _path.push_back(_index);
                        _index = 0;
                    } else if (_page->header.is_root) {
                        _path.clear();
                        _pages.clear();
                        return;
                    } else {
                        _index = _path.back() + 1;
                        _path.pop_back();
                        _pages.pop_back();
                        _page = _pages.back();
                    }
                    show_path();
                } while (!_page->header.is_leaf);
                _key = & _page->keys[_index];
                _value = & _page->values[_index];
                return;
            }
        }
    };
    inline cursor_t find(const key_t& key) {
        return cursor_t(this);
    }
    inline cursor_t begin() {
        return cursor_t(this);
    }
    static inline cursor_t end() {
        return cursor_t();
    }

    inline bool check() {
        key_t nullkey;
        return check(0, nullkey);
    }
    inline bool check(const size_t page_index, key_t& key) {
        const page_t& page = this->get_page(page_index);
        if (page.header.is_leaf) {
            for (uint32_t i=0, n=page.header.keys_count; i<n; i++) {
                if (key > page.keys[i]) {
                    return false;
                }
                key = page.keys[i];
            }
        } else {
            for (uint32_t i=0, n=page.header.keys_count; i<=n; i++) {
                if (!check(page.values[i], key) || page.values[i] == 0) {
                    return false;
                }
            }
        }
        return true;
    }

    inline void show(const size_t page_index=0, const size_t depth=0) {
        if (page_index == 0) {
            printf("BTREE\n");
            // debug("BTREE");
        }
        const page_t& page = this->get_page(page_index);
        std::string wrong_prefix(4 * (depth + 1), '|');
        std::string right_prefix(4 * (depth + 1), '.');
        key_t key;
        if (page.header.is_leaf) {
            for (uint32_t i=0, n=page.header.keys_count; i<n; i++) {
                const char* prefix = ((key > page.keys[i]) ? wrong_prefix : right_prefix).c_str();
                key = page.keys[i];
                printf("%s LEAF %-3u (%s)\n", prefix, page.values[i], page.keys[i]._data);
                // debug("%s LEAF %-3u (%s)", prefix, page.values[i], page.keys[i]._data);
            }
        } else {
            for (uint32_t i=0, n=page.header.keys_count; i<=n; i++) {
                key = page.keys[i];
                if (i == 0) {
                    const char* prefix = right_prefix.c_str();
                    printf("%s %u\n", prefix, page.values[i]);
                    // debug("%s %u", prefix, page.values[i]);
                } else {
                    const char* prefix = ((key > page.keys[i]) ? wrong_prefix : right_prefix).c_str();
                    printf("%s %-3u (%s)\n", prefix, page.values[i], page.keys[i - 1]._data);
                    // debug("%s %-3u (%s)", prefix, page.values[i], page.keys[i - 1]._data);
                }
                show(page.values[i], depth + 1);
            }
        }
    }

};

template <typename size_t, typename key_t, size_t reserve_size, size_t page_size, size_t pages_max_count>
const size_t BTree<size_t, key_t, reserve_size, page_size, pages_max_count>::max_keys_count = BTreePage<size_t, key_t, page_size>::max_keys_count;


#endif // __INCLUDED__BTree_hpp__
