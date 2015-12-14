#ifndef __INCLUDED__BTree_hpp__
#define __INCLUDED__BTree_hpp__


#include "DupaDB.hpp"
#include "FilePager.hpp"

#include <vector>
#include <map>
#include <set>

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
        bool is_leaf : sizeof(size_t);
        bool is_root : sizeof(size_t);
        size_t index;
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
        size_t keys_count = header.keys_count++;
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
    // debugging
    inline void show() {
        debug("IS%s LEAF / IS%s ROOT", header.is_leaf ? "" : " NOT", header.is_root ? "" : " NOT");
        debug("INDEX = %u, KEYS COUNT = %u", header.index, header.keys_count);
        if (!header.is_leaf) {
            debug("¤ -> %u", values[0]);
        }
        for (size_t k=0; k<header.keys_count; k++) {
            debug("`%s` -> %u", keys[k]._data, values[k + 1 - header.is_leaf]);
        }
    }
};

template <typename size_t, typename key_t, size_t page_size>
const size_t BTreePage<size_t, key_t, page_size>::max_keys_count = 15;
// const size_t BTreePage<size_t, key_t, page_size>::max_keys_count = (page_size - sizeof(header_t) - sizeof(size_t)) / (sizeof(key_t) + sizeof(size_t));


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

    inline page_t& new_page() {
        size_t page_index;
        page_t& page = this->get_page(page_index = this->header->page_count++);
        page.header = {
            .is_leaf = true,
            .is_root = false,
            .index = page_index,
            .keys_count = 0
        };
        return page;
    }

    inline void split(page_t& page, const size_t parent_index=0) {
        static const size_t split_left = max_keys_count / 2;
        static const size_t split_right = max_keys_count - split_left;
        const key_t& split_key = page.keys[split_left];
        if (page.header.is_leaf) {
            if (page.header.is_root) {
                // first new child
                page_t& child1 = new_page();
                memcpy(child1.keys, page.keys, split_left * sizeof(key_t));
                memcpy(child1.values, page.values, split_left * sizeof(size_t));
                child1.header.keys_count = split_left;
                // second new child
                page_t& child2 = new_page();
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
                // original page
                page.header.keys_count = split_left;
                // new sibling
                page_t& sibling = new_page();
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
                page_t& child1 = new_page();
                child1.header.is_leaf = false;
                child1.header.keys_count = split_left;
                memcpy(child1.keys, page.keys, split_left * sizeof(key_t));
                memcpy(child1.values, page.values, (split_left + 1) * sizeof(size_t));
                // second new child
                page_t& child2 = new_page();
                child2.header.is_leaf = false;
                child2.header.keys_count = split_right - 1;
                memcpy(child2.keys, page.keys + split_left + 1, (split_right - 1) * sizeof(key_t));
                memcpy(child2.values, page.values + split_left + 1, split_right * sizeof(size_t));
                // original
                page.header.keys_count = 1;
                page.keys[0] = split_key;
                page.values[0] = child1.header.index;
                page.values[1] = child2.header.index;
            } else {
                // new sibling
                page_t& sibling = new_page();
                sibling.header.is_leaf = false;
                sibling.header.keys_count = split_right - 1;
                memcpy(sibling.keys, page.keys + split_left + 1, (split_right - 1) * sizeof(key_t));
                memcpy(sibling.values, page.values + split_left + 1, split_right * sizeof(size_t));
                // original
                page.header.keys_count = split_left;
                // parent
                page_t& parent = this->get_page(parent_index);
                parent.insert(split_key, sibling.header.index);
            }
        }
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
        size_t _page_index;
        size_t _pages_indices[32];
        size_t _pages_indices_count;
        //
        size_t _index;
        size_t _indices[32];
        size_t _indices_count;
        //
        key_t* _key;
        size_t* _value;

        inline cursor_t() : _btree(NULL) {
            _page_index = -1;
            _index = -1;
        }
        inline cursor_t(BTree<size_t, key_t, reserve_size, page_size, pages_max_count>* btree) : _btree(btree) {
            _page_index = 0;
            _pages_indices_count = _indices_count = 0;
            _index = -1;
            while (true) {
                page_t* page = & _btree->get_page(_page_index);
                if (page->header.keys_count == 0) {
                    _page_index = -1;
                    return;
                }
                if (page->header.is_leaf) {
                    _key = page->keys;
                    _value = page->values;
                    _index = 0;
                    return;
                }
                _pages_indices[_pages_indices_count++] = _page_index;
                _indices[_indices_count++] = 0;
                _page_index = page->values[0];
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
            for (int i=0; i<_indices_count; i++) {
                snprintf(buffer, sizeof(buffer), "(%u-%u) ", _pages_indices[i], _indices[i]);
                result += buffer;
            }
            snprintf(buffer, sizeof(buffer), "(%u-%u)", _page_index, _index);
            result += buffer;
            warning("PATH: %s", result.c_str());
        }

        inline const bool operator != (const cursor_t& other) {
            return (_page_index != other._page_index) || (_index != other._index);
        }
        inline void operator ++ () {
            while (true) {
                page_t* page = & _btree->get_page(_page_index);
                if (++_index < page->header.keys_count) {
                    _key = page->keys + _index;
                    _value = page->values + _index;
                    return;
                }
                while (true) {
                    size_t max_index = page->header.keys_count;
                    if (!page->header.is_leaf) {
                        ++max_index;
                    }
                    if (_index < max_index) {
                        if (page->header.is_leaf) {
                            break;
                        }
                        _pages_indices[_pages_indices_count++] = _page_index;
                        _page_index = page->values[_index];
                        _indices[_indices_count++] = _index;
                        _index = 0;
                    } else if (_pages_indices_count) {
                        _page_index = _pages_indices[--_pages_indices_count];
                        _index = _indices[--_indices_count] + 1;
                    } else {
                        _page_index = -1;
                        _index = -1;
                        return;
                    }
                    page = & _btree->get_page(_page_index);
                }
                _key = page->keys + _index;
                _value = page->values + _index;
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
            // printf("BTREE\n");
            debug("BTREE");
        }
        const page_t& page = this->get_page(page_index);
        std::string wrong_prefix(4 * (depth + 1), '|');
        std::string right_prefix(4 * (depth + 1), '.');
        key_t key;
        if (page.header.is_leaf) {
            for (uint32_t i=0, n=page.header.keys_count; i<n; i++) {
                const char* prefix = ((key > page.keys[i]) ? wrong_prefix : right_prefix).c_str();
                key = page.keys[i];
                // printf("%s VALUE %-3u (%s)\n", prefix, page.values[i], page.keys[i]._data);
                debug("%s VALUE %-3u (%s)", prefix, page.values[i], page.keys[i]._data);
            }
        } else {
            for (uint32_t i=0, n=page.header.keys_count; i<=n; i++) {
                key = page.keys[i];
                if (i == 0) {
                    const char* prefix = right_prefix.c_str();
                    // printf("%s %u\n", prefix, page.values[i]);
                    debug("%s %u", prefix, page.values[i]);
                } else {
                    const char* prefix = ((key > page.keys[i]) ? wrong_prefix : right_prefix).c_str();
                    // printf("%s %-3u (%s)\n", prefix, page.values[i], page.keys[i - 1]._data);
                    debug("%s %-3u (%s)", prefix, page.values[i], page.keys[i - 1]._data);
                }
                show(page.values[i], depth + 1);
            }
        }
    }
    inline const bool show_check(const std::map<key_t, size_t>& key2value, bool show=false) {
        key_t previous_key;
        size_t count = 0;
        std::set<key_t> keys;
        for (auto it=begin(); it!=end(); ++it) {
            keys.insert(it.key());
            if (show) {
                it.show_path();
                debug("%-6u `%s` -> %u", count, it.key().data(), it.value());
            }
            count++;
            auto key2value_it = key2value.find(it.key());
            if (key2value_it == key2value.end()) {
                if (show) {
                    error("VALUE ERROR: `%s` should not be there", it.key()._data);
                } else {
                    show_check(key2value, true);
                    return false;
                }
            } else if (key2value_it->second != it.value()) {
                if (show) {
                    error("VALUE ERROR: `%s` != %u", it.key()._data, it.value());
                } else {
                    show_check(key2value, true);
                    return false;
                }
            }
            if (previous_key >= it.key()) {
                if (show) {
                    error("ORDER ERROR: %s >= %s", previous_key._data, it.key()._data);
                } else {
                    show_check(key2value, true);
                    return false;
                }
            }
            previous_key = it.key();
        }
        size_t expected_count = key2value.size();
        if (count != expected_count) {
            if (show) {
                error("COUNT ERROR: %u != %u", count, expected_count);
                for (auto it=key2value.begin(); it!=key2value.end(); it++) {
                    if (keys.find(it->first) == keys.end()) {
                        error("MISSING: %s", it->first._data);
                    }
                }
                for (auto it=keys.begin(); it!=keys.end(); it++) {
                    if (key2value.find(*it) == key2value.end()) {
                        error("INTRUDER: %s", it->_data);
                    }
                }
            } else {
                show_check(key2value, true);
                return false;
            }
        }
        return !show;
    }
    inline void show_pages() {
        for (size_t p=0; p<this->header->page_count; p++) {
            message("PAGE %u", p);
            this->get_page(p).show();
        }
    }
};

template <typename size_t, typename key_t, size_t reserve_size, size_t page_size, size_t pages_max_count>
const size_t BTree<size_t, key_t, reserve_size, page_size, pages_max_count>::max_keys_count = BTreePage<size_t, key_t, page_size>::max_keys_count;


#endif // __INCLUDED__BTree_hpp__
