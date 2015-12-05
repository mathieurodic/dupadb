#ifndef __INCLUDED__File_hpp__
#define __INCLUDED__File_hpp__


#include "util/logging.hpp"

#include <unordered_map>

#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


struct FileHandler {

    const char* _path;
    int _handle;
    int _size;
    int _reserve_size;

    inline FileHandler(const char* path, const int reserve_size) {
        _path = path;
        _handle = open(_path, O_RDWR | O_CREAT, 0666);
        _reserve_size = reserve_size;
        if (_handle == -1) {
            error("could not open: `%s`", _path);
            fatal("%s", strerror(errno));
        }
        struct stat stat;
        if (fstat(_handle, &stat) == -1) {
            error("error while reading stat for: `%s`", _path);
            fatal("%s", strerror(errno));
        }
        _size = stat.st_size;
    }
    inline ~FileHandler() {
        if (close(_handle) == -1) {
            fatal("could not close: `%s`", _path);
        }
    }

    inline const int& size() {
        return _size;
    }
    inline const int& reserve(const int max_size) {
        if (_size >= max_size) {
            return _size;
        }
        _size = max_size + (_reserve_size - max_size % _reserve_size);
        if (lseek(_handle, _size - 1, SEEK_SET) == -1) {
            fatal("error while seeking file: `%s`", _path);
        }
        static const char zero = '\0';
        if (write(_handle, &zero, 1) == 0) {
            fatal("error while resizing file: `%s`", _path);
        }
        return _size;
    }

};

template <typename mapped_t>
struct FileHandlerMap {

    FileHandler* _file_handler;
    mapped_t* _data;
    size_t _size;
    inline FileHandlerMap() : _file_handler(NULL), _data((mapped_t*) -1), _size(0) {}
    inline ~FileHandlerMap() {
        unset();
    }

    inline void set_handler(FileHandler& file_handler) {
        _file_handler = &file_handler;
    }

    inline const void unset() {
        if (_size) {
            munmap(_data, _size);
            _size = 0;
        }
    }
    inline const bool set(const int offset, const int size) {
        unset();
        if (size + offset > _file_handler->size()) {
            int new_size = size + offset;
            if (_file_handler->reserve(new_size) < new_size) {
                return false;
            }
        }
        _data = (mapped_t*) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, _file_handler->_handle, offset);;
        if (_data != (void*) -1) {
            _size = size;
            return true;
        }
        return false;
    }

    inline mapped_t* data() {
        return _data;
    }
};


template <
    typename header_t, typename size_t,
    size_t page_size, typename page_header_t, typename page_value_t,
    size_t pages_max_count
>
struct FilePager : FileHandler {

    // type definition for page
    struct page_t {
        page_header_t header;
        page_value_t values[(page_size - sizeof(page_header_t)) / sizeof(page_value_t)];
    };

    // mapped header
    FileHandlerMap<header_t> header_map;
    header_t* header;

    // other mapped pages
    FileHandlerMap<page_t> __page_maps[pages_max_count];
    size_t __page_maps_size;
    size_t _page_uses[pages_max_count];
    size_t _total_page_uses;
    std::unordered_map<size_t, FileHandlerMap<page_t>&> _page_maps;

    // constructor
    inline FilePager(const char* path, int reserve_size) : FileHandler(path, reserve_size) {
        if (reserve_size < page_size) {
            fatal("reserve_size should be greater than page_size; however, %u < %u", reserve_size, page_size);
        }
        auto is_new = (size() == 0);
        // initialize pages
        __page_maps_size = 0;
        memset(_page_uses, 0, sizeof(_page_uses));
        _total_page_uses = 0;
        // set file handler
        header_map.set_handler(*this);
        for (int i=0; i<pages_max_count; i++) {
            __page_maps[i].set_handler(*this);
        }
        // map header
        if (!header_map.set(0, sizeof(header_t))) {
            fatal("could not map header for: `%s`", _path);
        }
        header = header_map.data();
        // initialize header if necessary
        if (is_new) {
            header->set();
        }
        // check header
        if (!header->check()) {
            fatal("invalid header for: `%s`", _path);
        }
    }
    inline ~FilePager() {
        if (munmap(header, sizeof(header_t)) == -1) {
            fatal("error while unmapping header for: `%s`", _path);
        }
        debug("close file `%s`", _path);
    }

    // access pages
    inline page_t& get_page(size_t page_index) {
        // try to retrieve from cache
        auto it = _page_maps.find(page_index);
        if (it != _page_maps.end()) {
            return * it->second.data();
        }
        // find an unoccupied space
        size_t page_position;
        if (__page_maps_size < pages_max_count) {
            page_position = __page_maps_size++;
        } else {
            page_position = 0;
            size_t page_position_uses = _page_uses[0];
            for (size_t i=1; i<pages_max_count; i++) {
                if (_page_uses[i] < page_position_uses) {
                    page_position_uses = _page_uses[i];
                    page_position = i;
                }
            }
            _total_page_uses -= page_position_uses;
            _page_maps.erase(page_position);
        }
        // set the memory at this position
        _total_page_uses++;
        _page_uses[page_position]++;
        __page_maps[page_position].set((page_index + 1) * page_size, page_size);
        _page_maps.insert(
            std::pair<size_t, FileHandlerMap<page_t>&>(
                page_index,
                __page_maps[page_position]
            )
        );
        return * __page_maps[page_position].data();
    }

};


#endif // __INCLUDED__File_hpp__
