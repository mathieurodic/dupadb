#ifndef __INCLUDED__File_hpp__
#define __INCLUDED__File_hpp__


#include "util/logging.hpp"

#include <unordered_map>
#include <vector>

#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


template <typename size_t>
struct FileHandler {

    const char* _path;
    size_t _handle;
    size_t _size;
    size_t _reserve_size;

    inline FileHandler(const char* path, const size_t reserve_size) {
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

    inline const size_t& size() {
        return _size;
    }
    inline const size_t& reserve(const size_t max_size) {
        if (_size >= max_size) {
            return _size;
        }
        _size = max_size + (_reserve_size - max_size % _reserve_size);
        if (ftruncate(_handle, _size) == -1) {
            fatal("error while resizing file: `%s` (%s)", _path, strerror(errno));
        }
        return _size;
    }

};

template <typename size_t, typename mapped_t>
struct FileHandlerMap {

    FileHandler<size_t>* _file_handler;
    mapped_t* _data;
    size_t _size;
    inline FileHandlerMap() : _file_handler(NULL), _data((mapped_t*) -1), _size(0) {}
    inline ~FileHandlerMap() {
        unset();
    }

    inline void set_handler(FileHandler<size_t>& file_handler) {
        _file_handler = &file_handler;
    }

    inline const void unset() {
        if (_size) {
            if (munmap(_data, _size) == -1) {
                fatal("bleuargh");
            }
            _size = 0;
        }
    }
    inline const bool set(const size_t offset, const size_t size) {
        unset();
        size_t new_size = size + offset;
        if (new_size > _file_handler->size()) {
            if (_file_handler->reserve(new_size) < new_size) {
                return false;
            }
        }
        _data = (mapped_t*) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, _file_handler->_handle, offset);;
        if (_data != (void*) -1) {
            _size = size;
            return true;
        }
        fatal("could not map %u bytes from %s@%u", size, _file_handler->_path, offset);
        return false;
    }

    inline mapped_t* data() {
        return _data;
    }
};


template <
    typename header_t, typename size_t,
    size_t page_size, typename _page_t,
    size_t pages_max_count
>
struct FilePager : FileHandler<size_t> {

    // extend page...
    struct page_t : _page_t {
        inline void flush() {
            msync(this, sizeof(this), MS_SYNC);
        }
    };

    // mapped header
    FileHandlerMap<size_t, header_t> header_map;
    header_t* header;

    // other mapped pages
    FileHandlerMap<size_t, page_t> __page_maps[pages_max_count];
    size_t __page_maps_size;
    size_t _page_uses[pages_max_count];
    size_t _total_page_uses;
    std::unordered_map<size_t, FileHandlerMap<size_t, page_t>&> _page_maps;

    // constructor
    inline FilePager(const char* path, size_t reserve_size) : FileHandler<size_t>(path, reserve_size) {
        if (reserve_size < page_size) {
            fatal("reserve_size should be greater than page_size; however, %lu < %lu", (uint64_t)reserve_size, (uint64_t)page_size);
        }
        auto is_new = (this->size() == 0);
        // initialize pages
        __page_maps_size = 0;
        memset(_page_uses, 0, sizeof(_page_uses));
        _total_page_uses = 0;
        // set file handler
        header_map.set_handler(*this);
        for (size_t i=0; i<pages_max_count; i++) {
            __page_maps[i].set_handler(*this);
        }
        // map header
        if (!header_map.set(0, sizeof(header_t))) {
            fatal("could not map header for: `%s`", this->_path);
        }
        header = header_map.data();
        // initialize header if necessary
        if (is_new) {
            header->set();
        }
        // check header
        if (!header->check()) {
            fatal("invalid header for: `%s`", this->_path);
        }
    }
    inline ~FilePager() {
        if (munmap(header, sizeof(header_t)) == -1) {
            fatal("error while unmapping header for: `%s`", this->_path);
        }
        debug("close file `%s`", this->_path);
    }

    // access pages
    std::unordered_map<size_t, page_t*> pages;
    inline page_t& get_page(size_t page_index) {
        auto it = pages.find(page_index);
        if (it != pages.end()) {
            return * it->second;
        }
        page_t* page = (page_t*) malloc(page_size);
        memset(page, 0, page_size);
        pages.insert(std::pair<size_t, page_t*>(page_index, page));
        return * page;
    }

};


#endif // __INCLUDED__File_hpp__
